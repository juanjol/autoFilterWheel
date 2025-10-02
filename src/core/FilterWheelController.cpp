#include "FilterWheelController.h"
#include "../drivers/MotorDriverFactory.h"
#include "../encoders/AS5600Encoder.h"
#include "../config.h"
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
    , isCalibrated(true)
    , isMoving(false)
    , motorEnabled(false)
    , errorCode(0)
    , needsCalibration(false)
    , inCalibrationMode(false)
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

    // After splash screen, show initial state (always READY)
    if (displayManager) {
        delay(1500);  // Let splash screen display for a moment
        displayManager->showFilterWheelState(
            "READY",
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
    #if DEBUG_MODE
    Serial.print("[moveToPosition] Called with position: ");
    Serial.println(position);
    #endif

    if (!isValidPosition(position)) {
        #if DEBUG_MODE
        Serial.println("[moveToPosition] ERROR: Invalid position");
        #endif
        setError(1); // Invalid position
        return false;
    }

    if (isMoving) {
        #if DEBUG_MODE
        Serial.println("[moveToPosition] ERROR: System busy");
        #endif
        setError(2); // System busy
        return false;
    }

    targetPosition = position;
    bool success = false;

    // Show moving state
    if (displayManager) {
        displayManager->showFilterWheelState("MOVING", currentPosition, numFilters,
                                            getFilterName(currentPosition).c_str(), true);
    }

    // ENCODER-BASED CONTROL: Use angle feedback if encoder is available
    if (encoder && encoder->isAvailable()) {
        #if DEBUG_MODE
        Serial.println("[moveToPosition] Using ENCODER-BASED control");
        #endif

        // Convert target position to angle
        float targetAngle = positionToAngle(position);
        #if DEBUG_MODE
        Serial.print("[moveToPosition] Target angle: ");
        Serial.print(targetAngle, 2);
        Serial.println("°");
        #endif

        // Use encoder feedback for precise positioning
        success = moveToAngleWithFeedback(targetAngle, ANGLE_CONTROL_TOLERANCE);

        #if DEBUG_MODE
        if (success) {
            Serial.println("[moveToPosition] Encoder-based positioning succeeded");
        } else {
            Serial.println("[moveToPosition] WARNING: Encoder-based positioning failed, falling back to step-based");
        }
        #endif
    }

    // STEP-BASED CONTROL: Fallback if encoder not available or encoder control failed
    if (!success) {
        #if DEBUG_MODE
        Serial.println("[moveToPosition] Using STEP-BASED control (fallback)");
        #endif

        int steps = calculateStepsToPosition(position);
        #if DEBUG_MODE
        Serial.print("[moveToPosition] Calculated steps: ");
        Serial.println(steps);
        #endif

        if (steps == 0) {
            // Already at target position
            success = true;
        } else {
            // Apply backlash compensation
            steps = applyBacklashCompensation(steps);

            // Enable motor first
            motorDriver->enableMotor();

            // Execute movement
            #if DEBUG_MODE
            Serial.println("[moveToPosition] Starting motor movement...");
            #endif
            if (steps > 0) {
                #if DEBUG_MODE
                Serial.print("[moveToPosition] Moving forward ");
                Serial.print(steps);
                Serial.println(" steps");
                #endif
                motorDriver->stepForward(steps);
            } else if (steps < 0) {
                #if DEBUG_MODE
                Serial.print("[moveToPosition] Moving backward ");
                Serial.print(-steps);
                Serial.println(" steps");
                #endif
                motorDriver->stepBackward(-steps);  // stepBackward expects positive value
            }

            #if DEBUG_MODE
            Serial.println("[moveToPosition] Movement completed");
            #endif
            success = true;
        }
    }

    // Update position if movement was successful
    if (success) {
        currentPosition = targetPosition;

        // Save position
        if (configManager) {
            configManager->saveCurrentPosition(currentPosition);
        }

        // Verify position with encoder if available
        if (encoder && encoder->isAvailable()) {
            float currentAngle = encoder->getAngle();
            float targetAngle = positionToAngle(currentPosition);
            float error = abs(calculateAngularError(currentAngle, targetAngle));

            #if DEBUG_MODE
            Serial.print("[moveToPosition] Final verification - Current angle: ");
            Serial.print(currentAngle, 2);
            Serial.print("°, Target: ");
            Serial.print(targetAngle, 2);
            Serial.print("°, Error: ");
            Serial.print(error, 2);
            Serial.println("°");
            #endif

            if (error > ANGLE_TOLERANCE) {
                #if DEBUG_MODE
                Serial.println("[moveToPosition] WARNING: Position verification shows significant error");
                #endif
                needsCalibration = true;
            }
        }

        // Update display to show ready
        if (displayManager) {
            displayManager->showFilterWheelState("READY", currentPosition, numFilters,
                                                getFilterName(currentPosition).c_str());
        }
    } else {
        #if DEBUG_MODE
        Serial.println("[moveToPosition] ERROR: Movement failed");
        #endif
        setError(1); // Movement failed
    }

    return success;
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
    #if DEBUG_MODE
    Serial.println("========================================");
    Serial.println("[CALIBRATION] Starting calibration process");
    Serial.println("========================================");
    #endif

    setCurrentPosition(1);
    isCalibrated = true;

    if (configManager) {
        configManager->setCalibrated(true);
    }

    // If encoder is available, calibrate angle offset so position 1 = 0°
    if (encoder && encoder->isAvailable()) {
        // Read raw angle multiple times to get stable reading
        #if DEBUG_MODE
        Serial.println("[CALIBRATION] Reading encoder angle (averaging 5 samples)...");
        #endif
        float angleSum = 0;
        const int SAMPLES = 5;

        for (int i = 0; i < SAMPLES; i++) {
            float reading = encoder->getAngle();
            angleSum += reading;
            #if DEBUG_MODE
            Serial.print("[CALIBRATION] Sample ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(reading, 2);
            Serial.println("°");
            #endif
            delay(50);
        }

        float averageAngle = angleSum / SAMPLES;
        #if DEBUG_MODE
        Serial.print("[CALIBRATION] Average angle BEFORE offset: ");
        Serial.print(averageAngle, 2);
        Serial.println("°");
        #endif

        // IMPORTANT: First read the current offset to see what we're changing FROM
        float oldOffset = encoder->getAngleOffset();
        #if DEBUG_MODE
        Serial.print("[CALIBRATION] Old offset was: ");
        Serial.print(oldOffset, 2);
        Serial.println("°");
        #endif

        // Set offset so that current angle becomes 0° (position 1)
        // The new offset should be the RAW angle at this position
        // We need to add the old offset back to get the true raw angle
        float rawAngle = averageAngle + oldOffset;
        #if DEBUG_MODE
        Serial.print("[CALIBRATION] Calculated raw angle: ");
        Serial.print(rawAngle, 2);
        Serial.println("°");
        #endif

        encoder->setAngleOffset(rawAngle);

        #if DEBUG_MODE
        Serial.print("[CALIBRATION] Set NEW encoder offset to: ");
        Serial.print(rawAngle, 2);
        Serial.println("°");

        // Verify it was set
        float verifySet = encoder->getAngleOffset();
        Serial.print("[CALIBRATION] Verify encoder accepted offset: ");
        Serial.print(verifySet, 2);
        Serial.println("°");

        if (abs(verifySet - rawAngle) > 0.1f) {
            Serial.println("[CALIBRATION] ERROR: Encoder did not accept offset!");
        }
        #endif

        // Save offset to EEPROM
        if (configManager) {
            configManager->saveAngleOffset(rawAngle);
            #if DEBUG_MODE
            Serial.println("[CALIBRATION] Offset saved to EEPROM");

            // Verify EEPROM save
            float verifyEEPROM = configManager->loadAngleOffset();
            Serial.print("[CALIBRATION] Verify EEPROM saved: ");
            Serial.print(verifyEEPROM, 2);
            Serial.println("°");

            if (abs(verifyEEPROM - rawAngle) > 0.1f) {
                Serial.println("[CALIBRATION] ERROR: EEPROM did not save correctly!");
            }
            #endif
        } else {
            #if DEBUG_MODE
            Serial.println("[CALIBRATION] ERROR: ConfigManager is NULL!");
            #endif
        }

        // Verify calibration with multiple readings
        Serial.println("[CALIBRATION] Verifying calibration...");
        delay(100);

        float verifySum = 0;
        for (int i = 0; i < 3; i++) {
            float reading = encoder->getAngle();
            verifySum += reading;
            Serial.print("[CALIBRATION] Verify sample ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(reading, 2);
            Serial.println("°");
            delay(50);
        }

        float finalAngle = verifySum / 3;
        Serial.print("[CALIBRATION] Final verified angle: ");
        Serial.print(finalAngle, 2);
        Serial.println("° (should be close to 0°)");

        if (abs(finalAngle) > 2.0f) {
            #if DEBUG_MODE
            Serial.println("[CALIBRATION] WARNING: Calibration error > 2°");
            Serial.println("[CALIBRATION] This may indicate encoder noise or movement during calibration");
            #endif
        } else {
            #if DEBUG_MODE
            Serial.println("[CALIBRATION] ✓ Calibration successful!");
            #endif
        }
    }

    if (displayManager) {
        displayManager->showFilterWheelState("CALIBRATED", currentPosition, numFilters,
                                            getFilterName(currentPosition).c_str());
    }

    #if DEBUG_MODE
    Serial.println("========================================");
    Serial.println("[CALIBRATION] Calibration complete!");
    Serial.println("========================================");
    #endif
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
        float currentAngle = const_cast<EncoderInterface*>(encoder.get())->getAngle();
        float targetAngle = const_cast<FilterWheelController*>(this)->positionToAngle(currentPosition);
        float error = const_cast<FilterWheelController*>(this)->calculateAngularError(currentAngle, targetAngle);

        status += ",ANGLE=" + String(currentAngle, 2);
        status += ",TARGET_ANGLE=" + String(targetAngle, 2);
        status += ",ANGLE_ERROR=" + String(error, 2);
        status += ",CONTROL=ENCODER";
    } else {
        status += ",CONTROL=STEPS";
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
        &currentPosition, &numFilters, &isCalibrated, &isMoving,
        this  // Pass controller reference for calibration commands
    );

    // Register all commands
    commandHandlers->registerAllCommands(*commandProcessor);

    return true;
}

void FilterWheelController::loadSystemConfiguration() {
    if (!configManager) return;

    // Load basic configuration
    numFilters = configManager->loadFilterCount();
    currentPosition = configManager->loadCurrentPosition();
    // Note: isCalibrated is always true - system is always ready

    // Load motor configuration
    if (motorDriver && configManager->hasMotorConfig()) {
        auto motorConfig = configManager->loadMotorConfig();
        motorDriver->setSpeed(motorConfig.speed);
        motorDriver->setMaxSpeed(motorConfig.maxSpeed);
        motorDriver->setAcceleration(motorConfig.acceleration);
    }

    // Load encoder configuration
    if (encoder && encoder->isAvailable() && configManager->isCalibrated()) {
        float angleOffset = configManager->loadAngleOffset();
        encoder->setAngleOffset(angleOffset);
    }
}

void FilterWheelController::updateMotorMovement() {
    // Simplified for blocking movement

    // Check if encoder is available for position verification
    if (encoder && encoder->isAvailable()) {
        static unsigned long lastCheckTime = 0;
        unsigned long currentTime = millis();

        // Only check position every 5 seconds to avoid spam
        if (currentTime - lastCheckTime > 5000) {
            lastCheckTime = currentTime;

            float currentAngle = encoder->getAngle();

            // If we have AS5600, we can verify our position
            // This helps detect if the motor has slipped or lost steps
            uint8_t encoderPosition = angleToPosition(currentAngle);

            if (encoderPosition != currentPosition && !isMoving && !inCalibrationMode) {
                needsCalibration = true;
            }
        }
    }
}

uint8_t FilterWheelController::angleToPosition(float angle) {
    // Convert angle (0-360) to position (1-numFilters)
    if (angle < 0 || angle >= 360.0) {
        return 1; // Default to position 1 for invalid angles
    }

    // Calculate position based on angle
    float degreesPerPosition = 360.0f / numFilters;

    // Add half of degreesPerPosition for proper rounding
    uint8_t position = ((int)(angle + (degreesPerPosition / 2.0f)) / (int)degreesPerPosition) + 1;

    // Handle wraparound
    if (position > numFilters) {
        position = 1;
    }

    return position;
}

float FilterWheelController::positionToAngle(uint8_t position) {
    // Convert position (1-based) to angle (0-360)
    if (position < 1 || position > numFilters) {
        return 0.0f; // Default to 0° for invalid positions
    }

    // Position 1 = 0°, Position 2 = 72°, etc.
    float degreesPerPosition = 360.0f / numFilters;
    return (position - 1) * degreesPerPosition;
}

float FilterWheelController::calculateAngularError(float currentAngle, float targetAngle) {
    // Calculate the shortest angular distance, accounting for 360° wraparound
    float error = targetAngle - currentAngle;

    // Normalize to [-180, +180] range
    while (error > 180.0f) error -= 360.0f;
    while (error < -180.0f) error += 360.0f;

    return error;
}

int8_t FilterWheelController::determineRotationDirection(float currentAngle, float targetAngle) {
    float error = calculateAngularError(currentAngle, targetAngle);

    if (abs(error) < ANGLE_CONTROL_TOLERANCE) {
        return 0; // Already at target
    }

    // Positive error = need to rotate CW, negative = CCW
    return (error > 0) ? 1 : -1;
}

bool FilterWheelController::moveToAngleWithFeedback(float targetAngle, float tolerance) {
    if (!encoder || !encoder->isAvailable()) {
        #if DEBUG_MODE
        Serial.println("[PID] ERROR: Encoder not available");
        #endif
        return false;
    }

    if (!motorDriver) {
        #if DEBUG_MODE
        Serial.println("[PID] ERROR: Motor driver not initialized");
        #endif
        return false;
    }

    #if DEBUG_MODE
    Serial.println("========================================");
    Serial.print("[PID] Starting PID control to ");
    Serial.print(targetAngle, 2);
    Serial.print("° (tolerance: ");
    Serial.print(tolerance, 2);
    Serial.println("°)");
    Serial.println("========================================");
    #endif

    motorDriver->enableMotor();

    // PID Controller variables
    float integralSum = 0.0f;
    float previousError = 0.0f;
    bool success = false;
    int iteration = 0;

    // Constants for steps conversion
    const int stepsPerRevolution = 2048; // 28BYJ-48
    const float stepsPerDegree = stepsPerRevolution / 360.0f;

    while (iteration < ANGLE_CONTROL_MAX_ITERATIONS && !success) {
        // Read current angle from encoder
        float currentAngle = encoder->getAngle();
        if (currentAngle < 0) {
            #if DEBUG_MODE
            Serial.println("[PID] ERROR: Failed to read encoder angle");
            #endif
            return false;
        }

        // Calculate error (with wraparound handling)
        float error = calculateAngularError(currentAngle, targetAngle);

        // Check if we've reached target
        if (abs(error) <= tolerance) {
            // Wait for motor to settle completely before confirming
            delay(200);

            // Re-read angle to verify final position
            float finalAngle = encoder->getAngle();
            float finalError = calculateAngularError(finalAngle, targetAngle);

            #if DEBUG_MODE
            Serial.println("[PID] ✓ TARGET REACHED!");
            Serial.print("[PID] Final angle: ");
            Serial.print(finalAngle, 2);
            Serial.print("° (target: ");
            Serial.print(targetAngle, 2);
            Serial.print("°), Final error: ");
            Serial.print(finalError, 2);
            Serial.println("°");
            #endif

            // If error is still within tolerance after settling, accept it
            if (abs(finalError) <= tolerance) {
                success = true;
                break;
            } else {
                #if DEBUG_MODE
                Serial.println("[PID] Warning: Position drifted after settling, continuing...");
                #endif
                // Continue PID loop to correct
            }
        }

        // ============================================
        // PID CALCULATION
        // ============================================

        // Proportional term: directly proportional to error
        float proportional = ANGLE_PID_KP * error;

        // Integral term: accumulates error over time (anti-windup protection)
        integralSum += error;
        if (integralSum > ANGLE_PID_INTEGRAL_MAX) integralSum = ANGLE_PID_INTEGRAL_MAX;
        if (integralSum < -ANGLE_PID_INTEGRAL_MAX) integralSum = -ANGLE_PID_INTEGRAL_MAX;
        float integral = ANGLE_PID_KI * integralSum;

        // Derivative term: rate of change of error (dampens oscillation)
        float derivative = ANGLE_PID_KD * (error - previousError);

        // PID output (in steps)
        float pidOutput = proportional + integral + derivative;

        // Convert to integer steps
        int stepsNeeded = (int)pidOutput;

        // Apply output limits (prevent too large/small movements)
        if (abs(stepsNeeded) > ANGLE_PID_OUTPUT_MAX) {
            stepsNeeded = (stepsNeeded > 0) ? ANGLE_PID_OUTPUT_MAX : -ANGLE_PID_OUTPUT_MAX;
        }
        if (abs(stepsNeeded) < ANGLE_PID_OUTPUT_MIN && abs(error) > tolerance) {
            stepsNeeded = (stepsNeeded > 0) ? ANGLE_PID_OUTPUT_MIN : -ANGLE_PID_OUTPUT_MIN;
        }

        // Overshoot prevention: reduce steps when very close to target
        // This compensates for motor inertia and mechanical lag
        if (abs(error) < 5.0f) {
            // When error < 5°, use 70% of calculated steps to prevent overshoot
            stepsNeeded = (int)(stepsNeeded * 0.7f);
            if (abs(stepsNeeded) < ANGLE_PID_OUTPUT_MIN && abs(stepsNeeded) > 0) {
                stepsNeeded = (stepsNeeded > 0) ? ANGLE_PID_OUTPUT_MIN : -ANGLE_PID_OUTPUT_MIN;
            }
        }

        // Log PID values
        #if DEBUG_MODE
        Serial.print("[PID] Iter ");
        Serial.print(iteration + 1);
        Serial.print(": Angle=");
        Serial.print(currentAngle, 2);
        Serial.print("° Err=");
        Serial.print(error, 2);
        Serial.print("° | P=");
        Serial.print(proportional, 1);
        Serial.print(" I=");
        Serial.print(integral, 1);
        Serial.print(" D=");
        Serial.print(derivative, 1);
        Serial.print(" → ");
        Serial.print(stepsNeeded);
        Serial.print(" steps (");
        Serial.print(abs(stepsNeeded) / stepsPerDegree, 1);
        Serial.println("°)");
        #endif

        // Execute movement
        if (stepsNeeded > 0) {
            motorDriver->stepForward(abs(stepsNeeded));
        } else if (stepsNeeded < 0) {
            motorDriver->stepBackward(abs(stepsNeeded));
        }

        // Update previous error for derivative calculation
        previousError = error;

        // Settling time for mechanical stabilization
        delay(ANGLE_PID_SETTLING_TIME);

        iteration++;
    }

    if (!success) {
        #if DEBUG_MODE
        Serial.println("[PID] ✗ FAILED to reach target");
        Serial.print("[PID] Final error: ");
        Serial.print(previousError, 2);
        Serial.print("° after ");
        Serial.print(iteration);
        Serial.println(" iterations");
        #endif

        // Disable motor even on failure
        if (motorDriver) {
            motorDriver->disableMotor();
            #if DEBUG_MODE
            Serial.println("[PID] Motor disabled (failed positioning)");
            #endif
        }

        setError(1); // Positioning error
        return false;
    }

    #if DEBUG_MODE
    Serial.print("[PID] Success in ");
    Serial.print(iteration);
    Serial.println(" iterations");
    #endif

    // Disable motor after successful positioning
    if (motorDriver) {
        delay(MOTOR_DISABLE_DELAY); // Wait before disabling (from config.h)
        motorDriver->disableMotor();
        #if DEBUG_MODE
        Serial.println("[PID] Motor disabled (positioning complete)");
        #endif
    }

    return true;
}

void FilterWheelController::updateDisplay() {
    if (displayManager) {
        // Update display content based on current state
        const char* status;
        if (inCalibrationMode) {
            status = "CAL POS 1";
        } else if (needsCalibration) {
            status = "NEED CAL";
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
    // SIMPLIFIED: Fallback step-based control (only used when encoder unavailable)
    if (targetPos == currentPosition) {
        return 0;
    }

    // Use default steps per revolution - no calibration needed with encoder
    int stepsPerRevolution = 2048; // Default for 28BYJ-48
    int stepsPerFilter = stepsPerRevolution / numFilters;

    // Simple forward movement calculation
    int positionDiff;
    if (targetPos > currentPosition) {
        positionDiff = targetPos - currentPosition;
    } else {
        positionDiff = (numFilters - currentPosition) + targetPos;
    }

    return positionDiff * stepsPerFilter;
}

int FilterWheelController::applyBacklashCompensation(int steps) {
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

void FilterWheelController::startGuidedCalibration() {
    #if DEBUG_MODE
    Serial.println("Starting guided calibration...");
    #endif

    // Enter calibration mode
    inCalibrationMode = true;
    needsCalibration = false;

    // Move to where we think position 1 is
    currentPosition = 1;

    // Show calibration message on display
    if (displayManager) {
        displayManager->showFilterWheelState("CAL POS 1", currentPosition, numFilters,
                                            "Use SF/SB", false);
    }

    #if DEBUG_MODE
    Serial.println("Position wheel at filter 1, then use #CALCFM to confirm");
    #endif
}

void FilterWheelController::finishGuidedCalibration() {
    if (!inCalibrationMode) {
        #if DEBUG_MODE
        Serial.println("Not in calibration mode");
        #endif
        return;
    }

    // Get current encoder angle
    if (encoder && encoder->isAvailable()) {
        float currentAngle = encoder->getAngle();

        // This angle should now represent position 1
        // Calculate the offset needed so that position 1 = current angle
        float targetAngleForPos1 = 0.0f;  // Position 1 should be at 0 degrees
        float offset = currentAngle - targetAngleForPos1;

        // Normalize offset to 0-360 range
        while (offset < 0) offset += 360.0f;
        while (offset >= 360.0f) offset -= 360.0f;

        // Save the offset
        encoder->setAngleOffset(offset);
        if (configManager) {
            configManager->saveAngleOffset(offset);
        }

        #if DEBUG_MODE
        Serial.print("Calibration complete! Offset saved: ");
        Serial.print(offset, 2);
        Serial.println("°");
        #endif

        // Exit calibration mode
        inCalibrationMode = false;
        isCalibrated = true;

        // Update display
        if (displayManager) {
            displayManager->showFilterWheelState("READY", currentPosition, numFilters,
                                                getFilterName(currentPosition).c_str());
        }
    } else {
        #if DEBUG_MODE
        Serial.println("ERROR: No encoder available for calibration");
        #endif
        inCalibrationMode = false;
    }
}

bool FilterWheelController::needsCalibrationCheck() const {
    return needsCalibration;
}

bool FilterWheelController::isInCalibrationMode() const {
    return inCalibrationMode;
}