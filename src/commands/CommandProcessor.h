#pragma once

#include <Arduino.h>
#include <functional>

/**
 * Command execution result
 */
enum class CommandResult {
    SUCCESS,
    ERROR_UNKNOWN_COMMAND,
    ERROR_INVALID_FORMAT,
    ERROR_INVALID_PARAMETER,
    ERROR_SYSTEM_BUSY,
    ERROR_NOT_CALIBRATED,
    ERROR_MOTOR_TIMEOUT,
    ERROR_ENCODER_UNAVAILABLE
};

/**
 * Command handler function type
 * @param cmd The command string (without # prefix)
 * @param response Output parameter for response string
 * @return CommandResult indicating success or error type
 */
using CommandHandler = std::function<CommandResult(const String& cmd, String& response)>;

/**
 * Command processor for serial interface
 * Handles parsing and dispatching of serial commands
 */
class CommandProcessor {
private:
    String commandBuffer;
    bool debugMode;

    // Command categories and their handlers
    struct CommandMapping {
        const char* prefix;
        const char* description;
        CommandHandler handler;
    };

    static constexpr uint8_t MAX_COMMAND_MAPPINGS = 32;
    CommandMapping commandMappings[MAX_COMMAND_MAPPINGS];
    uint8_t numMappings;

public:
    CommandProcessor();

    /**
     * Initialize command processor
     */
    void init();

    /**
     * Process incoming serial data
     * Call this from main loop when serial data is available
     */
    void processSerialInput();

    /**
     * Execute a command directly
     * @param command Command string (with or without # prefix)
     * @param response Output response string
     * @return CommandResult
     */
    CommandResult executeCommand(const String& command, String& response);

    /**
     * Register a command handler
     * @param prefix Command prefix (e.g., "MP", "GP", "STATUS")
     * @param description Human-readable description
     * @param handler Function to handle the command
     */
    void registerCommand(const char* prefix, const char* description, CommandHandler handler);

    /**
     * Enable/disable debug mode
     * In debug mode, additional diagnostic information is sent
     */
    void setDebugMode(bool enabled);
    bool isDebugMode() const;

    /**
     * Send response to serial port
     * @param response Response string
     * @param isError Whether this is an error response
     */
    void sendResponse(const String& response, bool isError = false);

    /**
     * Send debug message (only if debug mode is enabled)
     */
    void sendDebugMessage(const String& message);

    /**
     * Get command result as error string
     */
    static const char* getErrorString(CommandResult result);

    /**
     * Show help information for all registered commands
     */
    void showHelp();

    /**
     * Get statistics about command processing
     */
    struct Statistics {
        uint32_t totalCommands;
        uint32_t successfulCommands;
        uint32_t errorCommands;
        uint32_t unknownCommands;
    };

    Statistics getStatistics() const;
    void resetStatistics();

private:
    Statistics stats;

    /**
     * Parse and clean command string
     */
    String parseCommand(const String& rawCommand);

    /**
     * Find command handler for given command
     */
    CommandHandler* findCommandHandler(const String& command);

    /**
     * Process a complete command
     */
    void processCommand(const String& command);

    /**
     * Validate command format
     */
    bool isValidCommand(const String& command);
};