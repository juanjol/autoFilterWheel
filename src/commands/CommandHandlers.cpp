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
    processor.registerCommand("DISPMODE", "Set display mode (0=minimal, 1=detailed)",
        [this](const String& cmd, String& response) { return handleSetDisplayMode(cmd, response); });

    processor.registerCommand("DISPLAY", "Get display information",
        [this](const String& cmd, String& response) { return handleGetDisplayInfo(cmd, response); });

    processor.registerCommand("BRIGHT", "Set display brightness (0-255)",
        [this](const String& cmd, String& response) { return handleSetBrightness(cmd, response); });

    processor.registerCommand("DISPON", "Turn display on",
        [this](const String& cmd, String& response) { return handleDisplayOn(cmd, response); });

    processor.registerCommand("DISPOFF", "Turn display off",
        [this](const String& cmd, String& response) { return handleDisplayOff(cmd, response); });

    processor.registerCommand("DISPPOWER", "Set display power mode (0=auto, 1=always on, 2=always off)",
        [this](const String& cmd, String& response) { return handleDisplayPowerMode(cmd, response); });

    processor.registerCommand("DISPTIMEOUT", "Set display auto-off timeout (0-65535 seconds, 0=never)",
        [this](const String& cmd, String& response) { return handleDisplayTimeout(cmd, response); });

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

    // Custom angle calibration commands
    processor.registerCommand("SETANG", "Set custom angle for position",
        [this](const String& cmd, String& response) { return handleSetCustomAngle(cmd, response); });

    processor.registerCommand("GETANG", "Get custom angle for position",
        [this](const String& cmd, String& response) { return handleGetCustomAngle(cmd, response); });

    processor.registerCommand("CLEARANG", "Clear all custom angles",
        [this](const String& cmd, String& response) { return handleClearCustomAngles(cmd, response); });

    processor.registerCommand("MEASREV", "Measure full 360° revolution",
        [this](const String& cmd, String& response) { return handleMeasureRevolution(cmd, response); });

    // Direction inversion commands
    processor.registerCommand("MINV0", "Set motor direction normal",
        [this](const String& cmd, String& response) { return handleSetMotorInversion(cmd, response); });

    processor.registerCommand("MINV1", "Set motor direction inverted",
        [this](const String& cmd, String& response) { return handleSetMotorInversion(cmd, response); });

    processor.registerCommand("GMINV", "Get motor direction inversion status",
        [this](const String& cmd, String& response) { return handleGetMotorInversion(cmd, response); });

    processor.registerCommand("ENCINV0", "Set encoder direction normal",
        [this](const String& cmd, String& response) { return handleSetEncoderInversion(cmd, response); });

    processor.registerCommand("ENCINV1", "Set encoder direction inverted",
        [this](const String& cmd, String& response) { return handleSetEncoderInversion(cmd, response); });

    processor.registerCommand("GENCINV", "Get encoder direction inversion status",
        [this](const String& cmd, String& response) { return handleGetEncoderInversion(cmd, response); });

    // Configuration retrieval command
    processor.registerCommand("GETCONFIG", "Get complete system configuration",
        [this](const String& cmd, String& response) { return handleGetConfig(cmd, response); });
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

    // Extract filter number between "SN" and ":"
    String filterNumStr = cmd.substring(2, colonPos);  // From position 2 (after "SN") to colon
    int filterNum = filterNumStr.toInt();

    if (filterNum == 0 && filterNumStr != "0") {
        // toInt() returned 0 but the string wasn't "0", meaning parse failed
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
        response += ",MOTOR_INV=" + String(motorDriver->isDirectionReversed() ? "1" : "0");

        // Add encoder inversion status if encoder is available
        if (encoder && encoder->isAvailable()) {
            response += ",ENC_INV=" + String(encoder->isDirectionInverted() ? "1" : "0");
        }
    } else {
        response = "MOTOR_CONFIG:SPEED=1000,MAX_SPEED=2000,ACCEL=500,DISABLE_DELAY=1000,STEPS_PER_REV=2048,MOTOR_INV=0,ENC_INV=0";
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

CommandResult CommandHandlers::handleSetDisplayMode(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse mode parameter: DISPMODE0 = minimal, DISPMODE1 = detailed
    if (cmd.length() > 8) {
        int mode = cmd.substring(8).toInt();

        if (mode < 0 || mode > 1) {
            response = "ERROR:Invalid mode (0=minimal, 1=detailed)";
            return CommandResult::ERROR_INVALID_PARAMETER;
        }

        displayManager->setDisplayMode(mode);
        response = "DISPMODE" + String(mode) + ":" + (mode == 0 ? "Minimal" : "Detailed");
        return CommandResult::SUCCESS;
    } else {
        // Toggle mode if no parameter provided
        uint8_t currentMode = displayManager->getDisplayMode();
        uint8_t newMode = (currentMode == 0) ? 1 : 0;
        displayManager->setDisplayMode(newMode);
        response = "DISPMODE" + String(newMode) + ":" + (newMode == 0 ? "Minimal" : "Detailed");
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
    response += ",Brightness=" + String(displayManager->getBrightness());

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetBrightness(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse brightness parameter: BRIGHT0-BRIGHT255
    if (cmd.length() > 6) {
        int brightness = cmd.substring(6).toInt();

        if (brightness < 0 || brightness > 255) {
            response = "ERROR:Invalid brightness (0-255)";
            return CommandResult::ERROR_INVALID_PARAMETER;
        }

        displayManager->setBrightness(brightness);
        response = "BRIGHT:" + String(brightness);
        return CommandResult::SUCCESS;
    } else {
        // Return current brightness if no parameter provided
        response = "BRIGHT:" + String(displayManager->getBrightness());
        return CommandResult::SUCCESS;
    }
}

CommandResult CommandHandlers::handleDisplayOn(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    displayManager->turnOn();
    response = "DISPON:OK";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleDisplayOff(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    displayManager->turnOff();
    response = "DISPOFF:OK";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleDisplayPowerMode(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse power mode parameter: DISPPOWER0-DISPPOWER2
    if (cmd.length() > 9) {
        int mode = cmd.substring(9).toInt();

        if (mode < 0 || mode > 2) {
            response = "ERROR:Invalid power mode (0=auto, 1=always on, 2=always off)";
            return CommandResult::ERROR_INVALID_PARAMETER;
        }

        displayManager->setPowerMode(mode);
        const char* modeStr[] = {"Auto", "AlwaysOn", "AlwaysOff"};
        response = "DISPPOWER:" + String(mode) + ":" + String(modeStr[mode]);
        return CommandResult::SUCCESS;
    } else {
        // Return current power mode if no parameter provided
        uint8_t currentMode = displayManager->getPowerMode();
        const char* modeStr[] = {"Auto", "AlwaysOn", "AlwaysOff"};
        response = "DISPPOWER:" + String(currentMode) + ":" + String(modeStr[currentMode]);
        return CommandResult::SUCCESS;
    }
}

CommandResult CommandHandlers::handleDisplayTimeout(const String& cmd, String& response) {
    if (!displayManager) {
        response = "ERROR:Display not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse timeout parameter: DISPTIMEOUT0-DISPTIMEOUT65535
    if (cmd.length() > 11) {
        int timeout = cmd.substring(11).toInt();

        if (timeout < 0 || timeout > 65535) {
            response = "ERROR:Invalid timeout (0-65535 seconds)";
            return CommandResult::ERROR_INVALID_PARAMETER;
        }

        displayManager->setAutoOffTimeout(timeout);
        if (timeout == 0) {
            response = "DISPTIMEOUT:" + String(timeout) + ":Never";
        } else {
            response = "DISPTIMEOUT:" + String(timeout) + ":Seconds";
        }
        return CommandResult::SUCCESS;
    } else {
        // Return current timeout if no parameter provided
        uint16_t currentTimeout = displayManager->getAutoOffTimeout();
        if (currentTimeout == 0) {
            response = "DISPTIMEOUT:" + String(currentTimeout) + ":Never";
        } else {
            response = "DISPTIMEOUT:" + String(currentTimeout) + ":Seconds";
        }
        return CommandResult::SUCCESS;
    }
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

// ========================================
// CUSTOM ANGLE CALIBRATION HANDLERS
// ========================================

CommandResult CommandHandlers::handleSetCustomAngle(const String& cmd, String& response) {
    if (!encoder || !encoder->isAvailable()) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_ENCODER_UNAVAILABLE;
    }

    if (!configManager) {
        response = "ERROR:Config manager not available";
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // Parse command: SETANG[pos]:[angle]
    // Example: SETANG1:0.0, SETANG2:68.5
    int colonPos = cmd.indexOf(':');
    if (colonPos == -1) {
        response = "ERROR:Format is SETANG[pos]:[angle]";
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    // Extract position number from "SETANG[pos]"
    String posStr = cmd.substring(6, colonPos);  // Skip "SETANG"
    uint8_t position = posStr.toInt();

    if (position < 1 || position > *numFilters) {
        response = "ERROR:Invalid position (" + String(position) + "). Must be 1-" + String(*numFilters);
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // Extract angle from ":[angle]"
    String angleStr = cmd.substring(colonPos + 1);
    float angle = angleStr.toFloat();

    if (angle < 0.0f || angle >= 360.0f) {
        response = "ERROR:Invalid angle (" + String(angle, 2) + "). Must be 0-359.99";
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // Save custom angle
    configManager->saveCustomAngle(position, angle);

    response = "SETANG:Position " + String(position) + " set to " + String(angle, 2) + "°";
    #if DEBUG_MODE
    Serial.println("[SETANG] " + response);
    #endif

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetCustomAngle(const String& cmd, String& response) {
    if (!configManager) {
        response = "ERROR:Config manager not available";
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // Parse command: GETANG[pos] or GETANG (all angles)
    String posStr = cmd.substring(6);  // Skip "GETANG"

    if (posStr.length() == 0) {
        // Get all angles
        if (!configManager->hasCustomAngles()) {
            response = "GETANG:No custom angles configured (using uniform distribution)";
            return CommandResult::SUCCESS;
        }

        response = "GETANG:";
        for (uint8_t i = 1; i <= *numFilters; i++) {
            float angle = configManager->loadCustomAngle(i);
            if (angle >= 0.0f) {
                response += String(i) + "=" + String(angle, 2) + "°";
                if (i < *numFilters) response += ",";
            }
        }

        return CommandResult::SUCCESS;
    }

    // Get specific position angle
    uint8_t position = posStr.toInt();

    if (position < 1 || position > *numFilters) {
        response = "ERROR:Invalid position (" + String(position) + "). Must be 1-" + String(*numFilters);
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (!configManager->hasCustomAngles()) {
        // Calculate default uniform angle
        float degreesPerPosition = 360.0f / (*numFilters);
        float angle = (position - 1) * degreesPerPosition;
        response = "GETANG" + String(position) + ":" + String(angle, 2) + "° (default)";
    } else {
        float angle = configManager->loadCustomAngle(position);
        if (angle >= 0.0f) {
            response = "GETANG" + String(position) + ":" + String(angle, 2) + "° (custom)";
        } else {
            response = "ERROR:No angle stored for position " + String(position);
            return CommandResult::ERROR_INVALID_PARAMETER;
        }
    }

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleClearCustomAngles(const String& cmd, String& response) {
    if (!configManager) {
        response = "ERROR:Config manager not available";
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    configManager->clearCustomAngles();
    response = "CLEARANG:All custom angles cleared. Using uniform distribution.";
    #if DEBUG_MODE
    Serial.println("[CLEARANG] Custom angles cleared");
    #endif

    return CommandResult::SUCCESS;
}
CommandResult CommandHandlers::handleMeasureRevolution(const String& cmd, String& response) {
    // Verify encoder is available
    if (!encoder || !encoder->isAvailable()) {
        response = "ERROR:Encoder not available. Cannot measure revolution.";
        return CommandResult::ERROR_ENCODER_UNAVAILABLE;
    }

    // Verify motor driver is available
    if (!motorDriver) {
        response = "ERROR:Motor driver not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Check if system is busy
    if (*isMoving) {
        response = "ERROR:System busy";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    #if DEBUG_MODE
    Serial.println("========================================");
    Serial.println("[MEASREV] Starting full revolution measurement");
    Serial.println("[MEASREV] This will move the motor one complete 360° rotation");
    Serial.println("========================================");
    #endif

    // Read initial angle
    float startAngle = encoder->getAngle();
    if (startAngle < 0) {
        response = "ERROR:Failed to read encoder";
        return CommandResult::ERROR_ENCODER_UNAVAILABLE;
    }

    #if DEBUG_MODE
    Serial.print("[MEASREV] Starting angle: ");
    Serial.print(startAngle, 2);
    Serial.println("°");
    #endif

    // Calculate target angle (one full revolution from start)
    float targetAngle = startAngle;  // We want to return to the same angle (360° = 0° mod 360)

    // Enable motor
    motorDriver->enableMotor();
    *isMoving = true;

    // Reset motor step counter to 0
    long initialPosition = motorDriver->getCurrentPosition();
    #if DEBUG_MODE
    Serial.print("[MEASREV] Initial motor position: ");
    Serial.println(initialPosition);
    #endif

    // PID Controller variables
    float integralSum = 0.0f;
    float previousError = 0.0f;
    int iteration = 0;
    long totalSteps = 0;
    const float tolerance = ANGLE_CONTROL_TOLERANCE;

    // Track total rotation to ensure we complete at least 360°
    float totalRotation = 0.0f;
    float lastAngle = startAngle;
    bool fullRotationCompleted = false;

    while (iteration < ANGLE_CONTROL_MAX_ITERATIONS && !fullRotationCompleted) {
        // Read current angle
        float currentAngle = encoder->getAngle();
        if (currentAngle < 0) {
            *isMoving = false;
            response = "ERROR:Encoder read failed during movement";
            return CommandResult::ERROR_ENCODER_UNAVAILABLE;
        }

        // Track total rotation (handle wraparound)
        float angleDelta = currentAngle - lastAngle;
        if (angleDelta < -180.0f) angleDelta += 360.0f;  // Crossed 0° going forward
        if (angleDelta > 180.0f) angleDelta -= 360.0f;   // Crossed 0° going backward
        totalRotation += angleDelta;
        lastAngle = currentAngle;

        // Check if we've completed at least 360° and are back at start position
        if (totalRotation >= 350.0f) {  // 350° to account for tolerance
            float errorToStart = calculateAngularError(currentAngle, targetAngle);
            if (abs(errorToStart) <= tolerance) {
                delay(200);  // Let motor settle
                float finalAngle = encoder->getAngle();
                float finalError = calculateAngularError(finalAngle, targetAngle);

                if (abs(finalError) <= tolerance) {
                    fullRotationCompleted = true;
                    break;
                }
            }
        }

        // Calculate error for PID (we want to keep moving forward)
        float error;
        if (totalRotation < 350.0f) {
            // Still need to complete the rotation - move forward
            error = 360.0f - totalRotation;  // Remaining degrees to travel
        } else {
            // Close to completion - fine-tune to start position
            error = calculateAngularError(currentAngle, targetAngle);
        }

        // PID CALCULATION
        float proportional = ANGLE_PID_KP * error;

        integralSum += error;
        if (integralSum > ANGLE_PID_INTEGRAL_MAX) integralSum = ANGLE_PID_INTEGRAL_MAX;
        if (integralSum < -ANGLE_PID_INTEGRAL_MAX) integralSum = -ANGLE_PID_INTEGRAL_MAX;
        float integral = ANGLE_PID_KI * integralSum;

        float derivative = ANGLE_PID_KD * (error - previousError);
        float pidOutput = proportional + integral + derivative;

        int stepsNeeded = (int)pidOutput;

        // Apply output limits
        if (abs(stepsNeeded) > ANGLE_PID_OUTPUT_MAX) {
            stepsNeeded = (stepsNeeded > 0) ? ANGLE_PID_OUTPUT_MAX : -ANGLE_PID_OUTPUT_MAX;
        }
        if (abs(stepsNeeded) < ANGLE_PID_OUTPUT_MIN && abs(error) > tolerance) {
            stepsNeeded = (stepsNeeded > 0) ? ANGLE_PID_OUTPUT_MIN : -ANGLE_PID_OUTPUT_MIN;
        }

        // Log iteration
        #if DEBUG_MODE
        Serial.print("[MEASREV] Iter ");
        Serial.print(iteration + 1);
        Serial.print(": Angle=");
        Serial.print(currentAngle, 2);
        Serial.print("° Rotated=");
        Serial.print(totalRotation, 2);
        Serial.print("° Steps=");
        Serial.print(stepsNeeded);
        Serial.print(" TotalSteps=");
        Serial.println(totalSteps);
        #endif

        // Execute movement
        if (stepsNeeded != 0) {
            if (stepsNeeded > 0) {
                motorDriver->stepForward(stepsNeeded);
            } else {
                motorDriver->stepBackward(abs(stepsNeeded));
            }
            totalSteps += stepsNeeded;
        }

        previousError = error;
        iteration++;
        delay(ANGLE_PID_SETTLING_TIME);
    }

    // Calculate actual steps taken
    long finalPosition = motorDriver->getCurrentPosition();
    long measuredSteps = abs(finalPosition - initialPosition);

    *isMoving = false;

    #if DEBUG_MODE
    Serial.println("========================================");
    if (fullRotationCompleted) {
        Serial.println("[MEASREV] ✓ Full revolution completed successfully!");
        Serial.print("[MEASREV] Total steps measured: ");
        Serial.println(measuredSteps);
        Serial.print("[MEASREV] Total rotation: ");
        Serial.print(totalRotation, 2);
        Serial.println("°");
        Serial.print("[MEASREV] Final angle: ");
        Serial.print(encoder->getAngle(), 2);
        Serial.println("°");
        Serial.println("[MEASREV] ");
        Serial.println("[MEASREV] Update your config.h with:");
        Serial.print("[MEASREV] #define STEPS_PER_REVOLUTION ");
        Serial.println(measuredSteps);
        Serial.println("========================================");
    } else {
        Serial.println("[MEASREV] ✗ Failed to complete revolution");
        Serial.print("[MEASREV] Completed ");
        Serial.print(totalRotation, 2);
        Serial.println("° of rotation");
        Serial.println("========================================");
    }
    #endif

    if (fullRotationCompleted) {
        response = "MEASREV:SUCCESS Steps=" + String(measuredSteps) +
                   " Rotation=" + String(totalRotation, 2) + "° " +
                   "Update STEPS_PER_REVOLUTION to " + String(measuredSteps);
    } else {
        response = "MEASREV:INCOMPLETE Steps=" + String(measuredSteps) +
                   " Rotation=" + String(totalRotation, 2) + "°";
    }

    return fullRotationCompleted ? CommandResult::SUCCESS : CommandResult::ERROR_MOTOR_TIMEOUT;
}

float CommandHandlers::calculateAngularError(float current, float target) {
    float error = target - current;

    // Normalize to [-180, +180] range
    while (error > 180.0f) error -= 360.0f;
    while (error < -180.0f) error += 360.0f;

    return error;
}


// ========================================
// DIRECTION INVERSION HANDLERS
// ========================================

CommandResult CommandHandlers::handleSetMotorInversion(const String& cmd, String& response) {
    if (!motorDriver) {
        response = "ERROR:Motor driver not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    if (!configManager) {
        response = "ERROR:Config manager not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse command: MINV0 = normal, MINV1 = inverted
    bool inverted;
    if (cmd == "MINV0") {
        inverted = false;
    } else if (cmd == "MINV1") {
        inverted = true;
    } else {
        response = "ERROR:Use MINV0 (normal) or MINV1 (inverted)";
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    // Apply to motor driver
    motorDriver->setDirectionReversed(inverted);

    // Save to EEPROM
    configManager->saveMotorDirectionInverted(inverted);

    response = "MINV:" + String(inverted ? "Inverted" : "Normal");
    #if DEBUG_MODE
    Serial.println("[MINV] Motor direction set to " + String(inverted ? "inverted" : "normal"));
    #endif

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetMotorInversion(const String& cmd, String& response) {
    if (!motorDriver) {
        response = "ERROR:Motor driver not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    bool inverted = motorDriver->isDirectionReversed();
    response = "GMINV:" + String(inverted ? "1 (Inverted)" : "0 (Normal)");

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetEncoderInversion(const String& cmd, String& response) {
    if (!encoder || !encoder->isAvailable()) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_ENCODER_UNAVAILABLE;
    }

    if (!configManager) {
        response = "ERROR:Config manager not available";
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    // Parse command: ENCINV0 = normal, ENCINV1 = inverted
    bool inverted;
    if (cmd == "ENCINV0") {
        inverted = false;
    } else if (cmd == "ENCINV1") {
        inverted = true;
    } else {
        response = "ERROR:Use ENCINV0 (normal) or ENCINV1 (inverted)";
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    // Apply to encoder
    encoder->setDirectionInverted(inverted);

    // Save to EEPROM
    configManager->saveEncoderDirectionInverted(inverted);

    response = "ENCINV:" + String(inverted ? "Inverted" : "Normal");
    #if DEBUG_MODE
    Serial.println("[ENCINV] Encoder direction set to " + String(inverted ? "inverted" : "normal"));
    #endif

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetEncoderInversion(const String& cmd, String& response) {
    if (!encoder || !encoder->isAvailable()) {
        response = "ERROR:Encoder not available";
        return CommandResult::ERROR_ENCODER_UNAVAILABLE;
    }

    bool inverted = encoder->isDirectionInverted();
    response = "GENCINV:" + String(inverted ? "1 (Inverted)" : "0 (Normal)");

    return CommandResult::SUCCESS;
}

// ========================================
// CONFIGURATION RETRIEVAL COMMAND
// ========================================

CommandResult CommandHandlers::handleGetConfig(const String& cmd, String& response) {
    // Build multi-line configuration response
    response = "";

    // ========================================
    // SECTION 1: FILTER CONFIGURATION
    // ========================================
    response += "FILTER_COUNT:" + String(*numFilters) + "\n";

    for (uint8_t i = 1; i <= *numFilters; i++) {
        String filterName;
        if (configManager) {
            filterName = configManager->loadFilterName(i);
        } else {
            filterName = "Filter" + String(i);
        }
        response += "FILTER_NAME:" + String(i) + ":" + filterName + "\n";
    }

    // ========================================
    // SECTION 2: MOTOR CONFIGURATION
    // ========================================
    if (motorDriver) {
        response += "MOTOR_SPEED:" + String((int)motorDriver->getCurrentSpeed()) + "\n";
        response += "MOTOR_MAX_SPEED:" + String((int)motorDriver->getMaxSpeed()) + "\n";
        response += "MOTOR_ACCEL:" + String((int)motorDriver->getAcceleration()) + "\n";
        response += "MOTOR_DISABLE_DELAY:" + String(motorDriver->getDisableDelay()) + "\n";
        response += "MOTOR_STEPS_PER_REV:" + String(motorDriver->getStepsPerRevolution()) + "\n";
        response += "MOTOR_INV:" + String(motorDriver->isDirectionReversed() ? "1" : "0") + "\n";
    } else {
        // Default values if motor driver not available
        response += "MOTOR_SPEED:1000\n";
        response += "MOTOR_MAX_SPEED:2000\n";
        response += "MOTOR_ACCEL:500\n";
        response += "MOTOR_DISABLE_DELAY:1000\n";
        response += "MOTOR_STEPS_PER_REV:2048\n";
        response += "MOTOR_INV:0\n";
    }

    // Encoder inversion
    if (encoder && encoder->isAvailable()) {
        response += "ENC_INV:" + String(encoder->isDirectionInverted() ? "1" : "0") + "\n";
    } else {
        response += "ENC_INV:0\n";
    }

    // ========================================
    // SECTION 3: DISPLAY CONFIGURATION
    // ========================================
    response += "DISPLAY_SIZE:128x64\n";

    if (displayManager) {
        response += "DISPLAY_ROTATION:" + String(displayManager->isRotated180() ? "1" : "0") + "\n";
        response += "DISPLAY_ENABLED:" + String(displayManager->isEnabled() ? "1" : "0") + "\n";
        response += "DISPLAY_BRIGHTNESS:" + String(displayManager->getBrightness()) + "\n";
        response += "DISPLAY_MODE:" + String(displayManager->getDisplayMode()) + "\n";
        response += "DISPLAY_POWER_MODE:" + String(displayManager->getPowerMode()) + "\n";
        response += "DISPLAY_TIMEOUT:" + String(displayManager->getAutoOffTimeout()) + "\n";
    } else {
        // Default values if display manager not available
        response += "DISPLAY_ROTATION:0\n";
        response += "DISPLAY_ENABLED:1\n";
        response += "DISPLAY_BRIGHTNESS:255\n";
        response += "DISPLAY_MODE:1\n";
        response += "DISPLAY_POWER_MODE:0\n";
        response += "DISPLAY_TIMEOUT:30\n";
    }

    // ========================================
    // SECTION 4: SYSTEM STATUS
    // ========================================
    response += "STATUS_POSITION:" + String(*currentPosition) + "\n";
    response += "STATUS_MOVING:" + String(*isMoving ? "1" : "0") + "\n";
    response += "STATUS_CALIBRATED:" + String(*isCalibrated ? "1" : "0") + "\n";

    // Get current angle from encoder if available
    if (encoder && encoder->isAvailable()) {
        float currentAngle = encoder->getAngle();
        char angleStr[8];
        dtostrf(currentAngle, 5, 2, angleStr);  // Format: XXX.XX
        response += "STATUS_ANGLE:" + String(angleStr) + "\n";
    } else {
        response += "STATUS_ANGLE:0.00\n";
    }

    // ========================================
    // SECTION 5: TERMINATOR
    // ========================================
    response += "CONFIG_END";

    return CommandResult::SUCCESS;
}
