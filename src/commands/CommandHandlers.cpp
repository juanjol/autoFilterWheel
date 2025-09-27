#include "CommandHandlers.h"
#include "../drivers/MotorDriver.h"
#include "../display/DisplayManager.h"
#include "../config/ConfigManager.h"
#include "../encoders/EncoderInterface.h"

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
    response = "DEVICE_ID:ESP32FW-" + String(*numFilters) + "POS-V1.0";
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
    response = "HELP:Use #COMMAND format. Available: GP,MP[X],SP[X],STATUS,CAL,ID,VER,GF,FC[X],GN[X],SN[X]:Name,STOP";
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