#include "FilterWheelController.h"
#include "../drivers/MotorDriverFactory.h"
#include "../encoders/AS5600Encoder.h"
#include <Wire.h>

// C++11 compatibility helper for make_unique
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_compat(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

FilterWheelController::FilterWheelController()
    : currentPosition(1)
    , numFilters(5)
    , targetPosition(1)
    , isCalibrated(false)
    , isMoving(false)
    , motorEnabled(false)
    , errorCode(0)
    , lastUpdate(0)
    , motorDisableTime(0)
    , motorDisablePending(false)
    , movementStartTime(0)
    , displayUpdateInterval(100)
    , motorDisableDelay(1000)
    , debugMode(false)
{
}

FilterWheelController::~FilterWheelController() {
}

bool FilterWheelController::init(MotorDriverType motorType) {
    // Initialize components in order
    if (!initializeMotorDriver(motorType)) {
        return false;
    }

    if (!initializeDisplay()) {
        return false;
    }

    if (!initializeEncoder()) {
        // Encoder is optional, so don't fail initialization
    }

    if (!initializeCommandSystem()) {
        return false;
    }

    // Initialize configuration manager
    configManager = make_unique_compat<ConfigManager>();
    configManager->init();

    // Load system configuration
    loadSystemConfiguration();

    // Show splash screen
    showSplashScreen();

    // After splash screen, show initial state
    if (displayManager) {
        delay(1500);  // Let splash screen display for a moment
        const char* status = isCalibrated ? "READY" : "NOT CAL";
        displayManager->showFilterWheelState(
            status,
            currentPosition,
            numFilters,
            getFilterName(currentPosition).c_str(),
            false
        );
    }

    return true;
}

void FilterWheelController::update() {
    unsigned long currentTime = millis();

    // Update motor movement
    updateMotorMovement();

    // Update display
    updateDisplay();

    // Update motor power management
    updateMotorPowerManagement();

    // Check movement timeout
    checkMovementTimeout();

    lastUpdate = currentTime;
}

void FilterWheelController::handleSerial() {
    if (commandProcessor) {
        commandProcessor->processSerialInput();
    }
}

bool FilterWheelController::moveToPosition(uint8_t position) {
    if (!isValidPosition(position)) {
        setError(1); // Invalid position
        return false;
    }

    if (isMoving) {
        setError(2); // System busy
        return false;
    }

    targetPosition = position;
    int steps = calculateStepsToPosition(position);

    if (steps == 0) {
        // Already at target position
        return true;
    }

    // Apply backlash compensation
    steps = applyBacklashCompensation(steps);

    // Start movement
    motorDriver->move(steps);
    isMoving = true;
    movementStartTime = millis();

    if (displayManager) {
        displayManager->showFilterWheelState("MOVING", currentPosition, numFilters,
                                            getFilterName(currentPosition).c_str(), true);
    }

    return true;
}

uint8_t FilterWheelController::getCurrentPosition() const {
    return currentPosition;
}

uint8_t FilterWheelController::getTargetPosition() const {
    return targetPosition;
}

bool FilterWheelController::isMotorMoving() const {
    return isMoving;
}

void FilterWheelController::emergencyStop() {
    if (motorDriver) {
        motorDriver->emergencyStop();
    }
    isMoving = false;
    clearError();
}

void FilterWheelController::setCurrentPosition(uint8_t position) {
    if (isValidPosition(position)) {
        currentPosition = position;
        targetPosition = position;

        if (motorDriver) {
            motorDriver->setCurrentPosition(0); // Reset step counter
        }

        if (configManager) {
            configManager->saveCurrentPosition(position);
        }
    }
}

void FilterWheelController::calibrateHome() {
    setCurrentPosition(1);
    isCalibrated = true;

    if (configManager) {
        configManager->setCalibrated(true);
    }

    if (displayManager) {
        displayManager->showFilterWheelState("CALIBRATED", currentPosition, numFilters,
                                            getFilterName(currentPosition).c_str());
    }
}

String FilterWheelController::getFilterName(uint8_t filterIndex) const {
    if (configManager && filterIndex >= 1 && filterIndex <= numFilters) {
        return configManager->loadFilterName(filterIndex);
    }
    return String("Filter ") + String(filterIndex);
}

String FilterWheelController::getSystemStatus() const {
    String status = "STATUS:POS=" + String(currentPosition);
    status += ",MOVING=" + String(isMoving ? "YES" : "NO");
    status += ",CAL=" + String(isCalibrated ? "YES" : "NO");
    status += ",ERROR=" + String(errorCode);

    if (encoder && encoder->isAvailable()) {
        status += ",ANGLE=" + String(encoder->getAngle(), 1);
    }

    return status;
}

void FilterWheelController::showSplashScreen() {
    if (displayManager) {
        displayManager->showVersionInfo("1.0.0", motorDriver ? motorDriver->getDriverName() : "Unknown");
    }
}

bool FilterWheelController::initializeMotorDriver(MotorDriverType motorType) {
    motorDriver = MotorDriverFactory::createDriver(motorType);
    return motorDriver != nullptr;
}

bool FilterWheelController::initializeDisplay() {
    displayManager = make_unique_compat<DisplayManager>(128, 64, &Wire, -1, 5);
    return displayManager->init(0x3C);
}

bool FilterWheelController::initializeEncoder() {
    encoder = make_unique_compat<AS5600Encoder>(&Wire);
    return encoder->init();
}

bool FilterWheelController::initializeCommandSystem() {
    commandProcessor = make_unique_compat<CommandProcessor>();
    commandProcessor->init();

    // Create command handlers and register them
    commandHandlers = make_unique_compat<CommandHandlers>(
        motorDriver.get(), displayManager.get(), configManager.get(), encoder.get(),
        &currentPosition, &numFilters, &isCalibrated, &isMoving
    );

    // Register all commands
    commandHandlers->registerAllCommands(*commandProcessor);

    return true;
}

void FilterWheelController::loadSystemConfiguration() {
    if (!configManager) return;

    // Load basic configuration
    numFilters = configManager->loadFilterCount();
    isCalibrated = configManager->isCalibrated();
    currentPosition = configManager->loadCurrentPosition();

    // Load motor configuration
    if (motorDriver && configManager->hasMotorConfig()) {
        auto motorConfig = configManager->loadMotorConfig();
        motorDriver->setSpeed(motorConfig.speed);
        motorDriver->setMaxSpeed(motorConfig.maxSpeed);
        motorDriver->setAcceleration(motorConfig.acceleration);
    }

    // Load direction configuration
    if (motorDriver && configManager->hasDirectionConfig()) {
        auto dirConfig = configManager->loadDirectionConfig();
        motorDriver->setDirectionReversed(dirConfig.reverseDirection);
    }

    // Load encoder configuration
    if (encoder && encoder->isAvailable() && configManager->isCalibrated()) {
        float angleOffset = configManager->loadAngleOffset();
        encoder->setAngleOffset(angleOffset);
    }
}

void FilterWheelController::updateMotorMovement() {
    if (!motorDriver || !isMoving) {
        return;
    }

    bool stillRunning = motorDriver->run();

    if (!stillRunning) {
        // Movement completed
        isMoving = false;
        currentPosition = targetPosition;

        // Save position
        if (configManager) {
            configManager->saveCurrentPosition(currentPosition);
        }

        // Update display
        if (displayManager) {
            displayManager->showFilterWheelState("READY", currentPosition, numFilters,
                                                getFilterName(currentPosition).c_str());
        }

        // Schedule motor disable
        motorDisablePending = true;
        motorDisableTime = millis() + motorDisableDelay;
    }
}

void FilterWheelController::updateDisplay() {
    if (displayManager) {
        // Update display content based on current state
        const char* status;
        if (!isCalibrated) {
            status = "NOT CAL";
        } else if (isMoving) {
            status = "MOVING";
        } else if (errorCode != 0) {
            status = "ERROR";
        } else {
            status = "READY";
        }

        // Update the display with current state
        displayManager->showFilterWheelState(
            status,
            currentPosition,
            numFilters,
            getFilterName(currentPosition).c_str(),
            isMoving
        );

        // Process any display updates
        displayManager->update();
    }
}

void FilterWheelController::updateMotorPowerManagement() {
    if (motorDisablePending && millis() >= motorDisableTime) {
        if (motorDriver) {
            motorDriver->disableMotor();
        }
        motorDisablePending = false;
        motorEnabled = false;
    }
}

int FilterWheelController::calculateStepsToPosition(uint8_t targetPos) {
    if (targetPos == currentPosition) {
        return 0;
    }

    // Get steps per filter from configuration or default calculation
    int stepsPerRevolution = 2048; // Default for 28BYJ-48
    if (configManager && configManager->hasRevolutionCalibration()) {
        stepsPerRevolution = configManager->loadRevolutionCalibration();
    }

    int stepsPerFilter = stepsPerRevolution / numFilters;

    // Calculate movement in unidirectional mode (always forward)
    int positionDiff;
    if (targetPos > currentPosition) {
        positionDiff = targetPos - currentPosition;
    } else {
        positionDiff = (numFilters - currentPosition) + targetPos;
    }

    return positionDiff * stepsPerFilter;
}

int FilterWheelController::applyBacklashCompensation(int steps) {
    if (configManager && configManager->hasBacklashCalibration()) {
        uint8_t backlashSteps = configManager->loadBacklashCalibration();
        // Apply backlash compensation logic here
        // This is simplified - real implementation would track direction
        return steps + backlashSteps;
    }
    return steps;
}

bool FilterWheelController::isValidPosition(uint8_t position) const {
    return position >= 1 && position <= numFilters;
}

void FilterWheelController::checkMovementTimeout() {
    if (isMoving && (millis() - movementStartTime) > 30000) { // 30 second timeout
        emergencyStop();
        setError(3); // Movement timeout
    }
}

void FilterWheelController::setError(uint8_t error) {
    errorCode = error;
    if (displayManager) {
        displayManager->showError(error, "System Error");
    }
}

void FilterWheelController::clearError() {
    errorCode = 0;
}

// Getters
uint8_t FilterWheelController::getFilterCount() const {
    return numFilters;
}

bool FilterWheelController::getIsCalibrated() const {
    return isCalibrated;
}

float FilterWheelController::getEncoderAngle() const {
    if (encoder && encoder->isAvailable()) {
        return encoder->getAngle();
    }
    return -1.0f;
}

bool FilterWheelController::isEncoderAvailable() const {
    return encoder && encoder->isAvailable();
}

uint8_t FilterWheelController::getErrorCode() const {
    return errorCode;
}

bool FilterWheelController::isDebugMode() const {
    return debugMode;
}

// Component access
MotorDriver* FilterWheelController::getMotorDriver() const {
    return motorDriver.get();
}

DisplayManager* FilterWheelController::getDisplayManager() const {
    return displayManager.get();
}

ConfigManager* FilterWheelController::getConfigManager() const {
    return configManager.get();
}

EncoderInterface* FilterWheelController::getEncoder() const {
    return encoder.get();
}

// Setters and other methods would be implemented similarly...
void FilterWheelController::setFilterCount(uint8_t count) {
    if (count >= 3 && count <= 8) {
        numFilters = count;
        if (configManager) {
            configManager->saveFilterCount(count);
        }
    }
}

void FilterWheelController::setFilterName(uint8_t filterIndex, const char* name) {
    if (configManager && isValidPosition(filterIndex)) {
        configManager->saveFilterName(filterIndex, name);
    }
}

void FilterWheelController::setDebugMode(bool enabled) {
    debugMode = enabled;
    if (commandProcessor) {
        commandProcessor->setDebugMode(enabled);
    }
}