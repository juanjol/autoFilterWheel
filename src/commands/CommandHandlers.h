#pragma once

#include "CommandProcessor.h"

// Forward declarations for dependencies
class MotorDriver;
class DisplayManager;
class ConfigManager;
class EncoderInterface;

/**
 * Command handlers for different categories of functionality
 * These handlers bridge the command processor with the actual system components
 */
// Forward declaration
class FilterWheelController;

class CommandHandlers {
private:
    // Component references (injected via constructor)
    MotorDriver* motorDriver;
    DisplayManager* displayManager;
    ConfigManager* configManager;
    EncoderInterface* encoder;
    CommandProcessor* commandProcessor;
    FilterWheelController* controller;

    // System state references
    uint8_t* currentPosition;
    uint8_t* numFilters;
    bool* isCalibrated;
    bool* isMoving;

public:
    /**
     * Constructor - inject dependencies
     */
    CommandHandlers(MotorDriver* motor, DisplayManager* display,
                    ConfigManager* config, EncoderInterface* enc,
                    uint8_t* currentPos, uint8_t* filterCount,
                    bool* calibrated, bool* moving,
                    FilterWheelController* ctrl = nullptr);

    /**
     * Register all command handlers with the command processor
     */
    void registerAllCommands(CommandProcessor& processor);

    // ========================================
    // BASIC POSITION COMMANDS
    // ========================================

    /**
     * Get current position - GP
     */
    CommandResult handleGetPosition(const String& cmd, String& response);

    /**
     * Move to position - MP[1-X]
     */
    CommandResult handleMoveToPosition(const String& cmd, String& response);

    /**
     * Set current position - SP[1-X]
     */
    CommandResult handleSetPosition(const String& cmd, String& response);

    /**
     * Emergency stop - STOP
     */
    CommandResult handleEmergencyStop(const String& cmd, String& response);

    /**
     * Get system status - STATUS
     */
    CommandResult handleGetStatus(const String& cmd, String& response);

    // ========================================
    // FILTER CONFIGURATION COMMANDS
    // ========================================

    /**
     * Get filter count - GF
     */
    CommandResult handleGetFilterCount(const String& cmd, String& response);

    /**
     * Set filter count - FC[3-8]
     */
    CommandResult handleSetFilterCount(const String& cmd, String& response);

    /**
     * Get filter name - GN[1-X] or GN (all names)
     */
    CommandResult handleGetFilterName(const String& cmd, String& response);

    /**
     * Set filter name - SN[1-X]:Name
     */
    CommandResult handleSetFilterName(const String& cmd, String& response);

    // ========================================
    // MOTOR CONFIGURATION COMMANDS
    // ========================================

    /**
     * Set motor speed - MS[X]
     */
    CommandResult handleSetMotorSpeed(const String& cmd, String& response);

    /**
     * Set max motor speed - MXS[X]
     */
    CommandResult handleSetMaxMotorSpeed(const String& cmd, String& response);

    /**
     * Set motor acceleration - MA[X]
     */
    CommandResult handleSetMotorAcceleration(const String& cmd, String& response);

    /**
     * Set motor disable delay - MDD[X]
     */
    CommandResult handleSetMotorDisableDelay(const String& cmd, String& response);

    /**
     * Get motor configuration - GMC
     */
    CommandResult handleGetMotorConfig(const String& cmd, String& response);

    /**
     * Reset motor configuration - RMC
     */
    CommandResult handleResetMotorConfig(const String& cmd, String& response);

    // ========================================
    // CALIBRATION COMMANDS
    // ========================================

    /**
     * Calibrate home position - CAL
     */
    CommandResult handleCalibrateHome(const String& cmd, String& response);

    /**
     * Start guided calibration - CALSTART
     */
    CommandResult handleStartGuidedCalibration(const String& cmd, String& response);

    /**
     * Confirm guided calibration - CALCFM
     */
    CommandResult handleConfirmGuidedCalibration(const String& cmd, String& response);

    // ========================================
    // CUSTOM ANGLE CALIBRATION COMMANDS
    // ========================================

    /**
     * Set custom angle for position - SETANG[pos]:[angle]
     * Example: SETANG1:0.0, SETANG2:68.5
     */
    CommandResult handleSetCustomAngle(const String& cmd, String& response);

    /**
     * Get custom angle for position - GETANG[pos]
     * Example: GETANG1, GETANG2, or GETANG (all angles)
     */
    CommandResult handleGetCustomAngle(const String& cmd, String& response);

    /**
     * Clear all custom angles - CLEARANG
     */
    CommandResult handleClearCustomAngles(const String& cmd, String& response);

    // ========================================
    // MANUAL CONTROL COMMANDS
    // ========================================

    /**
     * Step forward - SF[X]
     */
    CommandResult handleStepForward(const String& cmd, String& response);

    /**
     * Step backward - SB[X]
     */
    CommandResult handleStepBackward(const String& cmd, String& response);

    /**
     * Motor enable - ME
     */
    CommandResult handleMotorEnable(const String& cmd, String& response);

    /**
     * Motor disable - MD
     */
    CommandResult handleMotorDisable(const String& cmd, String& response);

    /**
     * Test motor directly - TESTMOTOR
     */
    CommandResult handleTestMotor(const String& cmd, String& response);

    // ========================================
    // SYSTEM INFO COMMANDS
    // ========================================

    /**
     * Get device ID - ID
     */
    CommandResult handleGetDeviceId(const String& cmd, String& response);

    /**
     * Get version - VER
     */
    CommandResult handleGetVersion(const String& cmd, String& response);

    /**
     * Get encoder angle - ANGLE (if encoder available)
     */
    CommandResult handleGetEncoderAngle(const String& cmd, String& response);

    /**
     * Help command - HELP
     */
    CommandResult handleHelp(const String& cmd, String& response);

    /**
     * Rotate display - ROTATE[0/1]
     */
    CommandResult handleRotateDisplay(const String& cmd, String& response);

    /**
     * Get display info - DISPLAY
     */
    CommandResult handleGetDisplayInfo(const String& cmd, String& response);

    /**
     * Get encoder status - ENCSTATUS
     */
    CommandResult handleGetEncoderStatus(const String& cmd, String& response);

    /**
     * Get rotation direction - ENCDIR
     */
    CommandResult handleGetRotationDirection(const String& cmd, String& response);

    /**
     * Get raw encoder debug info - ENCRAW
     */
    CommandResult handleGetEncoderRaw(const String& cmd, String& response);

    // ========================================
    // DIRECTION INVERSION COMMANDS
    // ========================================

    /**
     * Set motor direction inversion - MINV0/MINV1
     */
    CommandResult handleSetMotorInversion(const String& cmd, String& response);

    /**
     * Get motor direction inversion status - GMINV
     */
    CommandResult handleGetMotorInversion(const String& cmd, String& response);

    /**
     * Set encoder direction inversion - ENCINV0/ENCINV1
     */
    CommandResult handleSetEncoderInversion(const String& cmd, String& response);

    /**
     * Get encoder direction inversion status - GENCINV
     */
    CommandResult handleGetEncoderInversion(const String& cmd, String& response);

private:
    /**
     * Helper methods for parameter parsing
     */
    bool parseIntParameter(const String& cmd, const String& prefix, int& value);
    bool parseStringParameter(const String& cmd, const String& prefix, String& value);
    bool isValidPosition(uint8_t position);

    /**
     * Helper methods for movement
     */
    bool canExecuteMovement();
    void updateSystemState();
};