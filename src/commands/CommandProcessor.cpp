#include "CommandProcessor.h"

CommandProcessor::CommandProcessor()
    : commandBuffer("")
    , debugMode(false)
    , numMappings(0)
    , stats{0, 0, 0, 0}
{
}

void CommandProcessor::init() {
    Serial.begin(115200);
    commandBuffer.reserve(64);  // Reserve space for command buffer

    // Clear statistics
    resetStatistics();
}

void CommandProcessor::processSerialInput() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (commandBuffer.length() > 0) {
                processCommand(commandBuffer);
                commandBuffer = "";
            }
        } else if (c >= 32 && c <= 126) {  // Printable characters only
            commandBuffer += c;
        }
    }
}

CommandResult CommandProcessor::executeCommand(const String& command, String& response) {
    stats.totalCommands++;

    String cleanCommand = parseCommand(command);

    if (!isValidCommand(cleanCommand)) {
        stats.errorCommands++;
        response = "ERROR:INVALID_FORMAT";
        return CommandResult::ERROR_INVALID_FORMAT;
    }

    CommandHandler* handler = findCommandHandler(cleanCommand);
    if (!handler) {
        stats.unknownCommands++;
        response = "ERROR:UNKNOWN_COMMAND";
        return CommandResult::ERROR_UNKNOWN_COMMAND;
    }

    CommandResult result = (*handler)(cleanCommand, response);

    if (result == CommandResult::SUCCESS) {
        stats.successfulCommands++;
    } else {
        stats.errorCommands++;
        if (response.isEmpty()) {
            response = getErrorString(result);
        }
    }

    return result;
}

void CommandProcessor::registerCommand(const char* prefix, const char* description, CommandHandler handler) {
    if (numMappings < MAX_COMMAND_MAPPINGS) {
        commandMappings[numMappings] = {prefix, description, handler};
        numMappings++;
    }
}

void CommandProcessor::setDebugMode(bool enabled) {
    debugMode = enabled;
}

bool CommandProcessor::isDebugMode() const {
    return debugMode;
}

void CommandProcessor::sendResponse(const String& response, bool isError) {
    Serial.println(response);

    if (debugMode && !isError) {
        sendDebugMessage("Command processed successfully");
    }
}

void CommandProcessor::sendDebugMessage(const String& message) {
    if (debugMode) {
        Serial.print("Debug: ");
        Serial.println(message);
    }
}

const char* CommandProcessor::getErrorString(CommandResult result) {
    switch (result) {
        case CommandResult::SUCCESS:
            return "OK";
        case CommandResult::ERROR_UNKNOWN_COMMAND:
            return "ERROR:UNKNOWN_COMMAND";
        case CommandResult::ERROR_INVALID_FORMAT:
            return "ERROR:INVALID_FORMAT";
        case CommandResult::ERROR_INVALID_PARAMETER:
            return "ERROR:INVALID_PARAMETER";
        case CommandResult::ERROR_SYSTEM_BUSY:
            return "ERROR:SYSTEM_BUSY";
        case CommandResult::ERROR_NOT_CALIBRATED:
            return "ERROR:NOT_CALIBRATED";
        case CommandResult::ERROR_MOTOR_TIMEOUT:
            return "ERROR:MOTOR_TIMEOUT";
        case CommandResult::ERROR_ENCODER_UNAVAILABLE:
            return "ERROR:ENCODER_UNAVAILABLE";
        default:
            return "ERROR:UNKNOWN";
    }
}

void CommandProcessor::showHelp() {
    Serial.println("Available Commands:");
    Serial.println("==================");

    for (uint8_t i = 0; i < numMappings; i++) {
        Serial.print("#");
        Serial.print(commandMappings[i].prefix);
        Serial.print(" - ");
        Serial.println(commandMappings[i].description);
    }

    Serial.println();
    Serial.print("Total registered commands: ");
    Serial.println(numMappings);
}

String CommandProcessor::getHelpString() {
    String help = "HELP:Commands(";
    help += String(numMappings);
    help += "):";

    for (uint8_t i = 0; i < numMappings; i++) {
        if (i > 0) help += ",";
        help += commandMappings[i].prefix;
    }

    return help;
}

CommandProcessor::Statistics CommandProcessor::getStatistics() const {
    return stats;
}

void CommandProcessor::resetStatistics() {
    stats = {0, 0, 0, 0};
}

String CommandProcessor::parseCommand(const String& rawCommand) {
    String cleaned = rawCommand;
    cleaned.trim();
    cleaned.toUpperCase();

    // Remove # prefix if present
    if (cleaned.startsWith("#")) {
        cleaned = cleaned.substring(1);
    }

    return cleaned;
}

CommandHandler* CommandProcessor::findCommandHandler(const String& command) {
    // Find the longest matching prefix to avoid conflicts like CAL vs CALWIZ
    CommandHandler* bestMatch = nullptr;
    size_t bestMatchLength = 0;

    for (uint8_t i = 0; i < numMappings; i++) {
        String prefix = String(commandMappings[i].prefix);

        // Check if command matches this prefix
        if (command == prefix || command.startsWith(prefix)) {
            // Keep the longest match
            if (prefix.length() > bestMatchLength) {
                bestMatch = &commandMappings[i].handler;
                bestMatchLength = prefix.length();
            }
        }
    }

    return bestMatch;
}

void CommandProcessor::processCommand(const String& command) {
    String response;
    CommandResult result = executeCommand(command, response);

    bool isError = (result != CommandResult::SUCCESS);
    sendResponse(response, isError);

    if (debugMode) {
        String debugMsg = "Command: '" + command + "' -> Result: ";
        debugMsg += getErrorString(result);
        sendDebugMessage(debugMsg);
    }
}

bool CommandProcessor::isValidCommand(const String& command) {
    // Basic validation: command should not be empty and contain valid characters
    if (command.length() == 0) {
        return false;
    }

    // Commands should only contain alphanumeric characters, colons, underscores, dots, and minus signs
    for (size_t i = 0; i < command.length(); i++) {
        char c = command.charAt(i);
        if (!isAlphaNumeric(c) && c != ':' && c != '_' && c != '.' && c != '-') {
            return false;
        }
    }

    return true;
}