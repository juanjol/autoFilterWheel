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
class CommandHandlers {
private:
    // Component references (injected via constructor)
    MotorDriver* motorDriver;
    DisplayManager* displayManager;
    ConfigManager* configManager;
    EncoderInterface* encoder;

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
                    bool* calibrated, bool* moving);

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

    /**
     * Set direction mode - MDM[0-1]
     */
    CommandResult handleSetDirectionMode(const String& cmd, String& response);

    /**
     * Set reverse direction - MRV[0-1]
     */
    CommandResult handleSetReverseDirection(const String& cmd, String& response);

    /**
     * Get direction configuration - GDC
     */
    CommandResult handleGetDirectionConfig(const String& cmd, String& response);

    /**
     * Set steps per revolution - SPR[X]
     */
    CommandResult handleSetStepsPerRevolution(const String& cmd, String& response);

    /**
     * Get steps per revolution - GPR
     */
    CommandResult handleGetStepsPerRevolution(const String& cmd, String& response);

    // ========================================
    // CALIBRATION COMMANDS
    // ========================================

    /**
     * Calibrate home position - CAL
     */
    CommandResult handleCalibrateHome(const String& cmd, String& response);

    /**
     * Start revolution calibration - REVCAL
     */
    CommandResult handleStartRevolutionCalibration(const String& cmd, String& response);

    /**
     * Revolution calibration plus - RCP[X]
     */
    CommandResult handleRevolutionCalibrationPlus(const String& cmd, String& response);

    /**
     * Revolution calibration minus - RCM[X]
     */
    CommandResult handleRevolutionCalibrationMinus(const String& cmd, String& response);

    /**
     * Finish revolution calibration - RCFIN
     */
    CommandResult handleFinishRevolutionCalibration(const String& cmd, String& response);

    /**
     * Start backlash calibration - BLCAL
     */
    CommandResult handleStartBacklashCalibration(const String& cmd, String& response);

    /**
     * Backlash step - BLS[X]
     */
    CommandResult handleBacklashStep(const String& cmd, String& response);

    /**
     * Backlash mark - BLM
     */
    CommandResult handleBacklashMark(const String& cmd, String& response);

    /**
     * Finish backlash calibration - BLFIN
     */
    CommandResult handleFinishBacklashCalibration(const String& cmd, String& response);

    // ========================================
    // BACKLASH CONFIGURATION COMMANDS
    // ========================================

    /**
     * Set backlash steps - BLS[X]
     */
    CommandResult handleSetBacklashSteps(const String& cmd, String& response);

    /**
     * Get backlash configuration - BLG
     */
    CommandResult handleGetBacklashConfig(const String& cmd, String& response);

    /**
     * Set backlash enabled - BLE[0/1]
     */
    CommandResult handleSetBacklashEnabled(const String& cmd, String& response);

    /**
     * Set unidirectional mode - UNI[0/1]
     */
    CommandResult handleSetUnidirectionalMode(const String& cmd, String& response);

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
     * Step to position - ST[X]
     */
    CommandResult handleStepToPosition(const String& cmd, String& response);

    /**
     * Get step position - GST
     */
    CommandResult handleGetStepPosition(const String& cmd, String& response);

    /**
     * Motor enable - ME
     */
    CommandResult handleMotorEnable(const String& cmd, String& response);

    /**
     * Motor disable - MD
     */
    CommandResult handleMotorDisable(const String& cmd, String& response);

    /**
     * Start revolution calibration - REVCAL
     */
    CommandResult handleStartRevCalibration(const String& cmd, String& response);

    /**
     * Revolution calibration adjust plus - RCP
     */
    CommandResult handleRevCalAdjustPlus(const String& cmd, String& response);

    /**
     * Revolution calibration adjust minus - RCM
     */
    CommandResult handleRevCalAdjustMinus(const String& cmd, String& response);

    /**
     * Finish revolution calibration - RCFIN
     */
    CommandResult handleFinishRevCalibration(const String& cmd, String& response);

    /**
     * Go to step - ST[X]
     */
    CommandResult handleGoToStep(const String& cmd, String& response);

    /**
     * Get current step - GST
     */
    CommandResult handleGetCurrentStep(const String& cmd, String& response);

    /**
     * Enable motor - ME
     */
    CommandResult handleEnableMotor(const String& cmd, String& response);

    /**
     * Disable motor - MD
     */
    CommandResult handleDisableMotor(const String& cmd, String& response);

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