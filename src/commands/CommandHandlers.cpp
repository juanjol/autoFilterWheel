#include "CommandHandlers.h"
#include "../drivers/MotorDriver.h"
#include "../drivers/ULN2003Driver.h"
#include "../display/DisplayManager.h"
#include "../config/ConfigManager.h"
#include "../encoders/EncoderInterface.h"
#include "../core/FilterWheelController.h"
#include "../config.h"

CommandHandlers::CommandHandlers(MotorDriver* motor, DisplayManager* display,
                                 ConfigManager* config, EncoderInterface* enc,
                                 uint8_t* currentPos, uint8_t* filterCount,
                                 bool* calibrated, bool* moving,
                                 FilterWheelController* ctrl)
    : motorDriver(motor)
    , displayManager(display)
    , configManager(config)
    , encoder(enc)
    , commandProcessor(nullptr)
    , controller(ctrl)
    , currentPosition(currentPos)
    , numFilters(filterCount)
    , isCalibrated(calibrated)
    , isMoving(moving)
{
}

void CommandHandlers::registerAllCommands(CommandProcessor& processor) {
    // Store reference to processor for HELP command
    commandProcessor = &processor;
    // Basic position commands
    processor.registerCommand("GP", "Get current position",
        [this](const String& cmd, String& response) { return handleGetPosition(cmd, response); });

    processor.registerCommand("MP", "Move to position",
        [this](const String& cmd, String& response) { return handleMoveToPosition(cmd, response); });

    processor.registerCommand("SP", "Set current position",
        [this](const String& cmd, String& response) { return handleSetPosition(cmd, response); });

    processor.registerCommand("STOP", "Emergency stop",
        [this](const String& cmd, String& response) { return handleEmergencyStop(cmd, response); });

    processor.registerCommand("STATUS", "Get system status",
        [this](const String& cmd, String& response) { return handleGetStatus(cmd, response); });

    // System info commands
    processor.registerCommand("ID", "Get device ID",
        [this](const String& cmd, String& response) { return handleGetDeviceId(cmd, response); });

    processor.registerCommand("VER", "Get version",
        [this](const String& cmd, String& response) { return handleGetVersion(cmd, response); });

    processor.registerCommand("CAL", "Calibrate home position",
        [this](const String& cmd, String& response) { return handleCalibrateHome(cmd, response); });

    // Filter configuration
    processor.registerCommand("GF", "Get filter count",
        [this](const String& cmd, String& response) { return handleGetFilterCount(cmd, response); });

    processor.registerCommand("FC", "Set filter count",
        [this](const String& cmd, String& response) { return handleSetFilterCount(cmd, response); });

    processor.registerCommand("GN", "Get filter names",
        [this](const String& cmd, String& response) { return handleGetFilterName(cmd, response); });

    processor.registerCommand("SN", "Set filter name",
        [this](const String& cmd, String& response) { return handleSetFilterName(cmd, response); });

    processor.registerCommand("HELP", "Show help",
        [this](const String& cmd, String& response) { return handleHelp(cmd, response); });

    // Display Commands
    processor.registerCommand("ROTATE", "Rotate display 180 degrees",
        [this](const String& cmd, String& response) { return handleRotateDisplay(cmd, response); });

    processor.registerCommand("DISPLAY", "Get display information",
        [this](const String& cmd, String& response) { return handleGetDisplayInfo(cmd, response); });

    // Encoder Commands
    processor.registerCommand("ENCSTATUS", "Get encoder status",
        [this](const String& cmd, String& response) { return handleGetEncoderStatus(cmd, response); });

    processor.registerCommand("ENCDIR", "Get rotation direction",
        [this](const String& cmd, String& response) { return handleGetRotationDirection(cmd, response); });

    processor.registerCommand("ENCRAW", "Get raw encoder debug info",
        [this](const String& cmd, String& response) { return handleGetEncoderRaw(cmd, response); });

    // Motor Configuration Commands
    processor.registerCommand("GMC", "Get motor configuration",
        [this](const String& cmd, String& response) { return handleGetMotorConfig(cmd, response); });

    processor.registerCommand("MS", "Set motor speed",
        [this](const String& cmd, String& response) { return handleSetMotorSpeed(cmd, response); });

    processor.registerCommand("MXS", "Set max motor speed",
        [this](const String& cmd, String& response) { return handleSetMaxMotorSpeed(cmd, response); });

    processor.registerCommand("MA", "Set motor acceleration",
        [this](const String& cmd, String& response) { return handleSetMotorAcceleration(cmd, response); });

    processor.registerCommand("MDD", "Set motor disable delay",
        [this](const String& cmd, String& response) { return handleSetMotorDisableDelay(cmd, response); });

    processor.registerCommand("RMC", "Reset motor configuration",
        [this](const String& cmd, String& response) { return handleResetMotorConfig(cmd, response); });

    // Manual Step Commands
    processor.registerCommand("SF", "Step forward",
        [this](const String& cmd, String& response) { return handleStepForward(cmd, response); });

    processor.registerCommand("SB", "Step backward",
        [this](const String& cmd, String& response) { return handleStepBackward(cmd, response); });

    processor.registerCommand("ME", "Enable motor",
        [this](const String& cmd, String& response) { return handleMotorEnable(cmd, response); });

    processor.registerCommand("MD", "Disable motor",
        [this](const String& cmd, String& response) { return handleMotorDisable(cmd, response); });

    processor.registerCommand("TESTMOTOR", "Test motor directly",
        [this](const String& cmd, String& response) { return handleTestMotor(cmd, response); });

    // Guided calibration for encoder offset
    processor.registerCommand("CALSTART", "Start guided calibration",
        [this](const String& cmd, String& response) { return handleStartGuidedCalibration(cmd, response); });

    processor.registerCommand("CALCFM", "Confirm guided calibration",
        [this](const String& cmd, String& response) { return handleConfirmGuidedCalibration(cmd, response); });
}

CommandResult CommandHandlers::handleGetPosition(const String& cmd, String& response) {
    response = "P" + String(*currentPosition);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleMoveToPosition(const String& cmd, String& response) {
    // Only check if system is busy (moving), allow movement without calibration
    if (*isMoving) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    int position;
    if (!parseIntParameter(cmd, "MP", position)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (!isValidPosition(position)) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // Actually move to the position using the controller
    if (controller) {
        bool success = controller->moveToPosition(position);
        if (success) {
            response = "M" + String(position);
            return CommandResult::SUCCESS;
        } else {
            response = "ERROR:Movement failed";
            return CommandResult::ERROR_SYSTEM_BUSY;
        }
    } else {
        response = "ERROR:No controller";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }
}

CommandResult CommandHandlers::handleSetPosition(const String& cmd, String& response) {
    int position;
    if (!parseIntParameter(cmd, "SP", position)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (!isValidPosition(position)) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    *currentPosition = position;
    if (configManager) {
        configManager->saveCurrentPosition(position);
    }

    response = "S" + String(position);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleEmergencyStop(const String& cmd, String& response) {
    if (motorDriver) {
        motorDriver->emergencyStop();
    }
    *isMoving = false;
    response = "STOPPED";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetStatus(const String& cmd, String& response) {
    response = "STATUS:POS=" + String(*currentPosition);
    response += ",MOVING=" + String(*isMoving ? "YES" : "NO");
    response += ",CAL=" + String(*isCalibrated ? "YES" : "NO");

    if (encoder && encoder->isAvailable()) {
        response += ",ANGLE=" + String(encoder->getAngle(), 1);
    }

    response += ",ERROR=0";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetDeviceId(const String& cmd, String& response) {
    response = "DEVICE_ID:ESP32_FILTER_WHEEL_V1";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetVersion(const String& cmd, String& response) {
    response = "VERSION:" + String(FIRMWARE_VERSION);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleCalibrateHome(const String& cmd, String& response) {
    // Call the actual calibration function in the controller
    if (controller) {
        controller->calibrateHome();
        response = "CALIBRATED";
        return CommandResult::SUCCESS;
    } else {
        response = "ERROR:No controller";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }
}

CommandResult CommandHandlers::handleGetFilterCount(const String& cmd, String& response) {
    response = "F" + String(*numFilters);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetFilterCount(const String& cmd, String& response) {
    if (cmd.length() < 3) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    int count = cmd.substring(2).toInt();
    if (count < MIN_FILTER_COUNT || count > MAX_FILTER_COUNT) {
        response = "ERROR:Count must be " + String(MIN_FILTER_COUNT) + "-" + String(MAX_FILTER_COUNT);
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    *numFilters = count;
    if (configManager) {
        configManager->saveFilterCount(count);
    }

    response = "FC" + String(count);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetFilterName(const String& cmd, String& response) {
    if (cmd == "GN") {
        // Get all filter names
        response = "NAMES:";
        for (uint8_t i = 1; i <= *numFilters; i++) {
            if (i > 1) response += ",";
            if (configManager) {
                response += configManager->loadFilterName(i);
            } else {
                response += "Filter" + String(i);
            }
        }
    } else {
        // Get specific filter name
        int filterNum;
        if (!parseIntParameter(cmd, "GN", filterNum)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }

        if (!isValidPosition(filterNum)) {
            return CommandResult::ERROR_INVALID_PARAMETER;
        }

        String name;
        if (configManager) {
            name = configManager->loadFilterName(filterNum);
        } else {
            name = "Filter" + String(filterNum);
        }

        response = "N" + String(filterNum) + ":" + name;
    }

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetFilterName(const String& cmd, String& response) {
    int colonPos = cmd.indexOf(':');
    if (colonPos == -1 || cmd.length() < 4) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    int filterNum;
    if (!parseIntParameter(cmd, "SN", filterNum)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (!isValidPosition(filterNum)) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    String name = cmd.substring(colonPos + 1);
    if (name.length() == 0 || name.length() > 15) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (configManager) {
        configManager->saveFilterName(filterNum, name.c_str());
    }

    response = "SN" + String(filterNum) + ":" + name;
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleHelp(const String& cmd, String& response) {
    if (commandProcessor) {
        response = commandProcessor->getHelpString();
    } else {
        response = "HELP:CommandProcessor not available";
    }
    return CommandResult::SUCCESS;
}

// Helper methods
bool CommandHandlers::parseIntParameter(const String& cmd, const String& prefix, int& value) {
    if (!cmd.startsWith(prefix)) {
        return false;
    }

    String paramStr = cmd.substring(prefix.length());
    if (paramStr.length() == 0) {
        return false;
    }

    value = paramStr.toInt();
    return true;
}

bool CommandHandlers::isValidPosition(uint8_t position) {
    return position >= 1 && position <= *numFilters;
}

bool CommandHandlers::canExecuteMovement() {
    return !*isMoving && *isCalibrated;
}

// ========================================
// MOTOR CONFIGURATION COMMANDS
// ========================================

CommandResult CommandHandlers::handleGetMotorConfig(const String& cmd, String& response) {
    if (motorDriver) {
        response = "MOTOR_CONFIG:SPEED=" + String(motorDriver->getCurrentSpeed());
        response += ",MAX_SPEED=" + String(motorDriver->getMaxSpeed());
        response += ",ACCEL=" + String(motorDriver->getAcceleration());
        response += ",DISABLE_DELAY=" + String(motorDriver->getDisableDelay());
        response += ",STEPS_PER_REV=" + String(motorDriver->getStepsPerRevolution());
    } else {
        response = "MOTOR_CONFIG:SPEED=1000,MAX_SPEED=2000,ACCEL=500,DISABLE_DELAY=1000,STEPS_PER_REV=2048";
    }
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetMotorSpeed(const String& cmd, String& response) {
    int speed;
    if (!parseIntParameter(cmd, "MS", speed)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (speed < 50 || speed > 3000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setSpeed(speed);
        if (configManager) {
            configManager->saveMotorSpeed(speed);
        }
    }

    response = "MS" + String(speed);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetMaxMotorSpeed(const String& cmd, String& response) {
    int maxSpeed;
    if (!parseIntParameter(cmd, "MXS", maxSpeed)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (maxSpeed < 100 || maxSpeed > 5000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setMaxSpeed(maxSpeed);
        if (configManager) {
            configManager->saveMaxMotorSpeed(maxSpeed);
        }
    }

    response = "MXS" + String(maxSpeed);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetMotorAcceleration(const String& cmd, String& response) {
    int accel;
    if (!parseIntParameter(cmd, "MA", accel)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (accel < 50 || accel > 2000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setAcceleration(accel);
        if (configManager) {
            configManager->saveMotorAcceleration(accel);
        }
    }

    response = "MA" + String(accel);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetMotorDisableDelay(const String& cmd, String& response) {
    int delay;
    if (!parseIntParameter(cmd, "MDD", delay)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (delay < 500 || delay > 10000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setDisableDelay(delay);
        if (configManager) {
            configManager->saveMotorDisableDelay(delay);
        }
    }

    response = "MDD" + String(delay);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleResetMotorConfig(const String& cmd, String& response) {
    if (motorDriver) {
        motorDriver->resetToDefaults();
        if (configManager) {
            configManager->resetMotorConfiguration();
        }
    }

    response = "MOTOR_CONFIG_RESET";
    return CommandResult::SUCCESS;
}

// ========================================
// MANUAL STEP COMMANDS
// ========================================

CommandResult CommandHandlers::handleStepForward(const String& cmd, String& response) {
    if (!canExecuteMovement()) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    int steps = 1; // Default to 1 step
    if (cmd.length() > 2) {
        if (!parseIntParameter(cmd, "SF", steps)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }
    }

    if (steps < MIN_MANUAL_STEPS || steps > MAX_MANUAL_STEPS) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        // Enable motor first
        motorDriver->enableMotor();
        motorDriver->stepForward(steps);
        // Activate movement flag so motor runs
        if (isMoving) {
            *isMoving = true;
        }
    }

    response = "SF" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleStepBackward(const String& cmd, String& response) {
    if (!canExecuteMovement()) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    int steps = 1; // Default to 1 step
    if (cmd.length() > 2) {
        if (!parseIntParameter(cmd, "SB", steps)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }
    }

    if (steps < MIN_MANUAL_STEPS || steps > MAX_MANUAL_STEPS) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        // Enable motor first
        motorDriver->enableMotor();
        motorDriver->stepBackward(steps);
        // Activate movement flag so motor runs
        if (isMoving) {
            *isMoving = true;
        }
    }

    response = "SB" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleMotorEnable(const String& cmd, String& response) {
    if (motorDriver) {
        motorDriver->enableMotor();
    }

    response = "MOTOR_ENABLED";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleMotorDisable(const String& cmd, String& response) {
    if (motorDriver) {
        motorDriver->disableMotor();
    }

    response = "MOTOR_DISABLED";
    return CommandResult::SUCCESS;
}

// External function from test_motor.cpp
extern void testMotorDirect();

CommandResult CommandHandlers::handleTestMotor(const String& cmd, String& response) {
    response = "TESTMOTOR:Running direct pin test...";

    // Call the test function
    testMotorDirect();

    response += " Complete. Check LEDs and motor movement.";
    return CommandResult::SUCCESS;
}

// ========================================
// DISPLAY COMMANDS
// ========================================

CommandResult CommandHandlers::handleRotateDisplay(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse rotation parameter: ROTATE0 = normal, ROTATE1 = 180°
    if (cmd.length() > 6) {
        int rotation = cmd.substring(6).toInt();

        if (rotation < 0 || rotation > 1) {
            response = "ERROR:Invalid rotation (0=normal, 1=180°)";
            return CommandResult::ERROR_SYSTEM_BUSY;
        }

        displayManager->setRotation(rotation == 1);
        response = "ROTATE" + String(rotation);
        return CommandResult::SUCCESS;
    } else {
        // Just toggle rotation if no parameter provided
        bool currentRotation = displayManager->isRotated180();
        displayManager->setRotation(!currentRotation);
        response = "ROTATE" + String(displayManager->isRotated180() ? 1 : 0);
        return CommandResult::SUCCESS;
    }
}

CommandResult CommandHandlers::handleGetDisplayInfo(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    response = "DISPLAY:";
    response += "Size=" + String(displayManager->getWidth()) + "x" + String(displayManager->getHeight());
    response += ",Rotation=" + String(displayManager->isRotated180() ? "180°" : "Normal");
    response += ",Enabled=" + String(displayManager->isEnabled() ? "Yes" : "No");
    response += ",Update=" + String(DISPLAY_UPDATE_INTERVAL) + "ms";

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetEncoderStatus(const String& cmd, String& response) {
    if (!encoder) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    if (!encoder->isAvailable()) {
        response = "ENCSTATUS:Not connected";
        return CommandResult::SUCCESS;
    }

    float angle = encoder->getAngle();
    uint16_t rawValue = encoder->getRawValue();
    int8_t direction = encoder->getRotationDirection();
    bool healthy = encoder->isHealthy();
    float offset = encoder->getAngleOffset();

    // Calculate expected angle for current position
    float expectedAngle = 0.0f;
    if (controller) {
        expectedAngle = controller->positionToAngle(*currentPosition);
    }
    float error = angle - expectedAngle;
    // Normalize error to [-180, 180]
    while (error > 180.0f) error -= 360.0f;
    while (error < -180.0f) error += 360.0f;

    response = "ENCSTATUS:";
    response += "Angle=" + String(angle, 2);
    response += ",Expected=" + String(expectedAngle, 2);
    response += ",Error=" + String(error, 2);
    response += ",Raw=" + String(rawValue);
    response += ",Offset=" + String(offset, 2);
    response += ",Dir=";
    if (direction == 1) {
        response += "CW";
    } else if (direction == -1) {
        response += "CCW";
    } else {
        response += "STOP";
    }
    response += ",Health=" + String(healthy ? "OK" : "FAULT");

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetRotationDirection(const String& cmd, String& response) {
    if (!encoder) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    if (!encoder->isAvailable()) {
        response = "ENCDIR:Not connected";
        return CommandResult::SUCCESS;
    }

    // Update angle reading to refresh direction
    encoder->getAngle();

    int8_t direction = encoder->getRotationDirection();

    response = "ENCDIR:";
    if (direction == 1) {
        response += "CW (+1)";
    } else if (direction == -1) {
        response += "CCW (-1)";
    } else {
        response += "STOPPED (0)";
    }

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetEncoderRaw(const String& cmd, String& response) {
    if (!encoder || !encoder->isAvailable()) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Get raw values
    uint16_t rawValue = encoder->getRawValue();
    float rawAngle = rawValue * (360.0f / 4096.0f); // AS5600 12-bit = 4096 counts
    float currentOffset = encoder->getAngleOffset();
    float adjustedAngle = encoder->getAngle();

    // Calculate what the angle SHOULD be after offset
    float calculatedAngle = rawAngle - currentOffset;
    while (calculatedAngle < 0) calculatedAngle += 360.0f;
    while (calculatedAngle >= 360.0f) calculatedAngle -= 360.0f;

    response = "ENCRAW:";
    response += "RawCounts=" + String(rawValue);
    response += ",RawAngle=" + String(rawAngle, 2);
    response += ",Offset=" + String(currentOffset, 2);
    response += ",Calculated=" + String(calculatedAngle, 2);
    response += ",Actual=" + String(adjustedAngle, 2);

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleStartGuidedCalibration(const String& cmd, String& response) {
    if (controller) {
        controller->startGuidedCalibration();
        response = "CALSTART:OK";
    } else {
        response = "ERROR:No controller";
    }
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleConfirmGuidedCalibration(const String& cmd, String& response) {
    if (controller) {
        controller->finishGuidedCalibration();
        response = "CALCFM:Complete";
    } else {
        response = "ERROR:No controller";
    }
    return CommandResult::SUCCESS;
}
