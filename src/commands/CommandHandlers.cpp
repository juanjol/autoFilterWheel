#include "CommandHandlers.h"
#include "../drivers/MotorDriver.h"
#include "../drivers/ULN2003Driver.h"
#include "../display/DisplayManager.h"
#include "../config/ConfigManager.h"
#include "../encoders/EncoderInterface.h"
#include "../config.h"

CommandHandlers::CommandHandlers(MotorDriver* motor, DisplayManager* display,
                                 ConfigManager* config, EncoderInterface* enc,
                                 uint8_t* currentPos, uint8_t* filterCount,
                                 bool* calibrated, bool* moving)
    : motorDriver(motor)
    , displayManager(display)
    , configManager(config)
    , encoder(enc)
    , currentPosition(currentPos)
    , numFilters(filterCount)
    , isCalibrated(calibrated)
    , isMoving(moving)
{
}

void CommandHandlers::registerAllCommands(CommandProcessor& processor) {
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

    processor.registerCommand("MDM", "Set direction mode",
        [this](const String& cmd, String& response) { return handleSetDirectionMode(cmd, response); });

    processor.registerCommand("MRV", "Set reverse direction",
        [this](const String& cmd, String& response) { return handleSetReverseDirection(cmd, response); });

    processor.registerCommand("GDC", "Get direction configuration",
        [this](const String& cmd, String& response) { return handleGetDirectionConfig(cmd, response); });

    processor.registerCommand("SPR", "Set steps per revolution",
        [this](const String& cmd, String& response) { return handleSetStepsPerRevolution(cmd, response); });

    processor.registerCommand("GPR", "Get steps per revolution",
        [this](const String& cmd, String& response) { return handleGetStepsPerRevolution(cmd, response); });

    // Manual Step Commands
    processor.registerCommand("SF", "Step forward",
        [this](const String& cmd, String& response) { return handleStepForward(cmd, response); });

    processor.registerCommand("SB", "Step backward",
        [this](const String& cmd, String& response) { return handleStepBackward(cmd, response); });

    processor.registerCommand("ST", "Step to absolute position",
        [this](const String& cmd, String& response) { return handleStepToPosition(cmd, response); });

    processor.registerCommand("GST", "Get step position",
        [this](const String& cmd, String& response) { return handleGetStepPosition(cmd, response); });

    processor.registerCommand("ME", "Enable motor",
        [this](const String& cmd, String& response) { return handleMotorEnable(cmd, response); });

    processor.registerCommand("MD", "Disable motor",
        [this](const String& cmd, String& response) { return handleMotorDisable(cmd, response); });

    // Calibration Commands
    processor.registerCommand("REVCAL", "Start revolution calibration",
        [this](const String& cmd, String& response) { return handleStartRevCalibration(cmd, response); });

    processor.registerCommand("RCP", "Add calibration steps",
        [this](const String& cmd, String& response) { return handleRevCalAdjustPlus(cmd, response); });

    processor.registerCommand("RCM", "Subtract calibration steps",
        [this](const String& cmd, String& response) { return handleRevCalAdjustMinus(cmd, response); });

    processor.registerCommand("RCFIN", "Finish revolution calibration",
        [this](const String& cmd, String& response) { return handleFinishRevCalibration(cmd, response); });

    // Backlash Calibration Commands
    processor.registerCommand("BLCAL", "Start backlash calibration",
        [this](const String& cmd, String& response) { return handleStartBacklashCalibration(cmd, response); });

    processor.registerCommand("BLS", "Backlash step",
        [this](const String& cmd, String& response) { return handleBacklashStep(cmd, response); });

    processor.registerCommand("BLM", "Backlash mark",
        [this](const String& cmd, String& response) { return handleBacklashMark(cmd, response); });

    processor.registerCommand("BLFIN", "Finish backlash calibration",
        [this](const String& cmd, String& response) { return handleFinishBacklashCalibration(cmd, response); });

    // Backlash Configuration Commands
    processor.registerCommand("BLS", "Set backlash steps",
        [this](const String& cmd, String& response) { return handleSetBacklashSteps(cmd, response); });

    processor.registerCommand("BLG", "Get backlash configuration",
        [this](const String& cmd, String& response) { return handleGetBacklashConfig(cmd, response); });

    processor.registerCommand("BLE", "Enable/disable backlash compensation",
        [this](const String& cmd, String& response) { return handleSetBacklashEnabled(cmd, response); });

    processor.registerCommand("UNI", "Set unidirectional mode",
        [this](const String& cmd, String& response) { return handleSetUnidirectionalMode(cmd, response); });
}

CommandResult CommandHandlers::handleGetPosition(const String& cmd, String& response) {
    response = "P" + String(*currentPosition);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleMoveToPosition(const String& cmd, String& response) {
    if (!canExecuteMovement()) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    int position;
    if (!parseIntParameter(cmd, "MP", position)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (!isValidPosition(position)) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // This would trigger movement in the actual controller
    response = "M" + String(position);
    return CommandResult::SUCCESS;
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
    response = "VERSION:1.0.0";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleCalibrateHome(const String& cmd, String& response) {
    *currentPosition = 1;
    *isCalibrated = true;

    if (configManager) {
        configManager->setCalibrated(true);
        configManager->saveCurrentPosition(1);
    }

    if (motorDriver) {
        motorDriver->setCurrentPosition(0);
    }

    response = "CALIBRATED";
    return CommandResult::SUCCESS;
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
    if (count < 3 || count > 8) {
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
    response = "HELP:Use #COMMAND format. Available: GP,MP[X],SP[X],STATUS,CAL,ID,VER,GF,FC[X],GN[X],SN[X]:Name,ROTATE[0/1],DISPLAY,STOP";
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

CommandResult CommandHandlers::handleSetDirectionMode(const String& cmd, String& response) {
    int mode;
    if (!parseIntParameter(cmd, "MDM", mode)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (mode != 0 && mode != 1) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setDirectionMode(mode == 1);
        if (configManager) {
            configManager->saveDirectionMode(mode == 1);
        }
    }

    response = "MDM" + String(mode);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetReverseDirection(const String& cmd, String& response) {
    int reverse;
    if (!parseIntParameter(cmd, "MRV", reverse)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (reverse != 0 && reverse != 1) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setReverseDirection(reverse == 1);
        if (configManager) {
            configManager->saveReverseDirection(reverse == 1);
        }
    }

    response = "MRV" + String(reverse);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetDirectionConfig(const String& cmd, String& response) {
    bool dirMode = false;
    bool revMode = false;

    if (motorDriver) {
        dirMode = motorDriver->getDirectionMode();
        revMode = motorDriver->getReverseDirection();
    }

    response = "DIRECTION_CONFIG:MODE=" + String(dirMode ? 1 : 0);
    response += ",REVERSE=" + String(revMode ? 1 : 0);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetStepsPerRevolution(const String& cmd, String& response) {
    int steps;
    if (!parseIntParameter(cmd, "SPR", steps)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (steps < 200 || steps > 8192) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setStepsPerRevolution(steps);
        if (configManager) {
            configManager->saveStepsPerRevolution(steps);
        }
    }

    response = "SPR" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetStepsPerRevolution(const String& cmd, String& response) {
    int steps = 2048; // Default value
    if (motorDriver) {
        steps = motorDriver->getStepsPerRevolution();
    }

    response = "SPR:" + String(steps);
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

    if (steps < 1 || steps > 1000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->stepForward(steps);
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

    if (steps < 1 || steps > 1000) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->stepBackward(steps);
    }

    response = "SB" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleStepToPosition(const String& cmd, String& response) {
    if (!canExecuteMovement()) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    int stepPos;
    if (!parseIntParameter(cmd, "ST", stepPos)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (stepPos < 0 || stepPos > 4096) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->goToStep(stepPos);
    }

    response = "ST" + String(stepPos);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetStepPosition(const String& cmd, String& response) {
    int stepPos = 0;
    if (motorDriver) {
        stepPos = motorDriver->getCurrentStep();
    }

    response = "STEP:" + String(stepPos);
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

// ========================================
// CALIBRATION COMMANDS
// ========================================

CommandResult CommandHandlers::handleStartRevCalibration(const String& cmd, String& response) {
    if (*isMoving) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    if (motorDriver) {
        motorDriver->startRevolutionCalibration();
    }

    response = "REV_CAL_STARTED";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleRevCalAdjustPlus(const String& cmd, String& response) {
    int steps = 1; // Default
    if (cmd.length() > 3) {
        if (!parseIntParameter(cmd, "RCP", steps)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }
    }

    if (steps < 1 || steps > 100) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->adjustRevolutionCalibration(steps);
    }

    response = "RCP" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleRevCalAdjustMinus(const String& cmd, String& response) {
    int steps = 1; // Default
    if (cmd.length() > 3) {
        if (!parseIntParameter(cmd, "RCM", steps)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }
    }

    if (steps < 1 || steps > 100) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->adjustRevolutionCalibration(-steps);
    }

    response = "RCM" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleFinishRevCalibration(const String& cmd, String& response) {
    if (motorDriver) {
        int totalSteps = motorDriver->finishRevolutionCalibration();
        if (configManager) {
            configManager->saveStepsPerRevolution(totalSteps);
        }
        response = "REV_CAL_COMPLETE:" + String(totalSteps);
    } else {
        response = "REV_CAL_COMPLETE:2048";
    }

    return CommandResult::SUCCESS;
}

// ========================================
// BACKLASH CALIBRATION COMMANDS
// ========================================

CommandResult CommandHandlers::handleStartBacklashCalibration(const String& cmd, String& response) {
    if (*isMoving) {
        return CommandResult::ERROR_SYSTEM_BUSY;
    }

    if (motorDriver) {
        motorDriver->startBacklashCalibration();
    }

    response = "BACKLASH_CAL_STARTED";
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleBacklashStep(const String& cmd, String& response) {
    int steps = 1; // Default
    if (cmd.length() > 3) {
        if (!parseIntParameter(cmd, "BLS", steps)) {
            return CommandResult::ERROR_INVALID_FORMAT;
        }
    }

    if (steps < 1 || steps > 50) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        int totalSteps = motorDriver->backlashTestStep(steps);
        response = "BLS" + String(steps) + " TOTAL:" + String(totalSteps);
    } else {
        response = "BLS" + String(steps) + " TOTAL:0";
    }

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleBacklashMark(const String& cmd, String& response) {
    if (motorDriver) {
        bool isForward = motorDriver->markBacklashMovement();
        int steps = motorDriver->getCurrentBacklashSteps();

        if (isForward) {
            response = "BLM_FORWARD:" + String(steps);
        } else {
            response = "BLM_BACKWARD:" + String(steps);
        }
    } else {
        response = "BLM_FORWARD:0";
    }

    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleFinishBacklashCalibration(const String& cmd, String& response) {
    if (motorDriver) {
        int backlashSteps = motorDriver->finishBacklashCalibration();
        if (configManager) {
            configManager->saveBacklashSteps(backlashSteps);
        }
        response = "BACKLASH_CAL_COMPLETE:" + String(backlashSteps);
    } else {
        response = "BACKLASH_CAL_COMPLETE:0";
    }

    return CommandResult::SUCCESS;
}

// ========================================
// BACKLASH CONFIGURATION COMMANDS
// ========================================

CommandResult CommandHandlers::handleSetBacklashSteps(const String& cmd, String& response) {
    int steps;
    if (!parseIntParameter(cmd, "BLS", steps)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (steps < 0 || steps > 100) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setBacklashSteps(steps);
        if (configManager) {
            configManager->saveBacklashSteps(steps);
        }
    }

    response = "BLS" + String(steps);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleGetBacklashConfig(const String& cmd, String& response) {
    int steps = 0;
    bool enabled = false;

    if (motorDriver) {
        steps = motorDriver->getBacklashSteps();
        enabled = motorDriver->isBacklashEnabled();
    }

    response = "BACKLASH_CONFIG:STEPS=" + String(steps);
    response += ",ENABLED=" + String(enabled ? "YES" : "NO");
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetBacklashEnabled(const String& cmd, String& response) {
    int enable;
    if (!parseIntParameter(cmd, "BLE", enable)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (enable != 0 && enable != 1) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    if (motorDriver) {
        motorDriver->setBacklashEnabled(enable == 1);
    }

    response = "BLE" + String(enable);
    return CommandResult::SUCCESS;
}

CommandResult CommandHandlers::handleSetUnidirectionalMode(const String& cmd, String& response) {
    int mode;
    if (!parseIntParameter(cmd, "UNI", mode)) {
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    if (mode != 0 && mode != 1) {
        return CommandResult::ERROR_INVALID_PARAMETER;
    }

    // For now, save the mode in ConfigManager - the actual driver implementation
    // will read this setting during initialization
    if (configManager) {
        configManager->saveDirectionMode(mode == 0);  // unidirectional = !bidirectional
    }

    response = "UNI" + String(mode);
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