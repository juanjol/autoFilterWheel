#pragma once

#include "../drivers/MotorDriver.h"
#include "../drivers/MotorDriverFactory.h"
#include "../display/DisplayManager.h"
#include "../commands/CommandProcessor.h"
#include "../commands/CommandHandlers.h"
#include "../config/ConfigManager.h"
#include "../encoders/EncoderInterface.h"
#include <memory>

/**
 * Main Filter Wheel Controller
 * Orchestrates all system components and provides high-level interface
 */
class FilterWheelController {
private:
    // Component instances
    std::unique_ptr<MotorDriver> motorDriver;
    std::unique_ptr<DisplayManager> displayManager;
    std::unique_ptr<CommandProcessor> commandProcessor;
    std::unique_ptr<CommandHandlers> commandHandlers;
    std::unique_ptr<ConfigManager> configManager;
    std::unique_ptr<EncoderInterface> encoder;

    // System state
    uint8_t currentPosition;
    uint8_t numFilters;
    uint8_t targetPosition;
    bool isCalibrated;
    bool isMoving;
    bool motorEnabled;
    uint8_t errorCode;
    bool needsCalibration;          // True when encoder mismatch detected
    bool inCalibrationMode;         // True during guided calibration

    // Timing and management
    unsigned long lastUpdate;
    unsigned long motorDisableTime;
    bool motorDisablePending;
    unsigned long movementStartTime;

    // Configuration
    uint16_t displayUpdateInterval;
    uint16_t motorDisableDelay;
    bool debugMode;

public:
    /**
     * Constructor
     */
    FilterWheelController();

    /**
     * Destructor
     */
    ~FilterWheelController();

    /**
     * Initialize the controller with specified motor driver type
     * @param motorType Type of motor driver to use
     * @return true if initialization successful
     */
    bool init(MotorDriverType motorType = MotorDriverType::ULN2003_28BYJ48);

    /**
     * Main update loop - call this from Arduino loop()
     */
    void update();

    /**
     * Handle serial communication
     */
    void handleSerial();

    // ========================================
    // HIGH-LEVEL MOVEMENT INTERFACE
    // ========================================

    /**
     * Move to specific filter position
     * @param position Target position (1-based)
     * @return true if movement started successfully
     */
    bool moveToPosition(uint8_t position);

    /**
     * Get current filter position
     */
    uint8_t getCurrentPosition() const;

    /**
     * Get target position (if moving)
     */
    uint8_t getTargetPosition() const;

    /**
     * Check if motor is currently moving
     */
    bool isMotorMoving() const;

    /**
     * Emergency stop
     */
    void emergencyStop();

    /**
     * Set current position without moving (calibration)
     */
    void setCurrentPosition(uint8_t position);

    /**
     * Start guided calibration mode
     */
    void startGuidedCalibration();

    /**
     * Finish guided calibration and save encoder offset
     */
    void finishGuidedCalibration();

    /**
     * Check if system needs calibration
     */
    bool needsCalibrationCheck() const;

    /**
     * Check if in calibration mode
     */
    bool isInCalibrationMode() const;

    // ========================================
    // CONFIGURATION INTERFACE
    // ========================================

    /**
     * Set number of filters
     */
    void setFilterCount(uint8_t count);

    /**
     * Get number of filters
     */
    uint8_t getFilterCount() const;

    /**
     * Set filter name
     */
    void setFilterName(uint8_t filterIndex, const char* name);

    /**
     * Get filter name
     */
    String getFilterName(uint8_t filterIndex) const;

    /**
     * Set motor parameters
     */
    void setMotorSpeed(float speed);
    void setMotorMaxSpeed(float maxSpeed);
    void setMotorAcceleration(float acceleration);

    /**
     * Get motor parameters
     */
    float getMotorSpeed() const;
    float getMotorMaxSpeed() const;
    float getMotorAcceleration() const;

    // ========================================
    // CALIBRATION INTERFACE
    // ========================================

    /**
     * Calibrate home position
     */
    void calibrateHome();

    /**
     * Check if system is calibrated
     */
    bool getIsCalibrated() const;

    /**
     * Start revolution calibration
     */
    bool startRevolutionCalibration();

    /**
     * Start backlash calibration
     */
    bool startBacklashCalibration();

    // ========================================
    // STATUS AND DIAGNOSTICS
    // ========================================

    /**
     * Get system status string
     */
    String getSystemStatus() const;

    /**
     * Get encoder angle (if available)
     */
    float getEncoderAngle() const;

    /**
     * Check if encoder is available
     */
    bool isEncoderAvailable() const;

    /**
     * Get error code
     */
    uint8_t getErrorCode() const;

    /**
     * Clear error code
     */
    void clearError();

    /**
     * Run system self-test
     */
    bool performSelfTest();

    // ========================================
    // SYSTEM CONTROL
    // ========================================

    /**
     * Enable/disable debug mode
     */
    void setDebugMode(bool enabled);

    /**
     * Get debug mode status
     */
    bool isDebugMode() const;

    /**
     * Show splash screen
     */
    void showSplashScreen();

    /**
     * Factory reset
     */
    void factoryReset();

    /**
     * Save current configuration
     */
    void saveConfiguration();

    /**
     * Load configuration from EEPROM
     */
    void loadConfiguration();

    // ========================================
    // COMPONENT ACCESS (for advanced users)
    // ========================================

    /**
     * Get direct access to components (use with caution)
     */
    MotorDriver* getMotorDriver() const;
    DisplayManager* getDisplayManager() const;
    ConfigManager* getConfigManager() const;
    EncoderInterface* getEncoder() const;

private:
    /**
     * Initialize components
     */
    bool initializeMotorDriver(MotorDriverType motorType);
    bool initializeDisplay();
    bool initializeEncoder();
    bool initializeCommandSystem();

    /**
     * Convert angle to filter position
     */
    uint8_t angleToPosition(float angle);

    /**
     * Load configuration from EEPROM
     */
    void loadSystemConfiguration();

    /**
     * Update motor state and handle movement
     */
    void updateMotorMovement();

    /**
     * Update display if needed
     */
    void updateDisplay();

    /**
     * Handle motor power management
     */
    void updateMotorPowerManagement();

    /**
     * Calculate steps for movement
     */
    int calculateStepsToPosition(uint8_t targetPos);

    /**
     * Apply backlash compensation
     */
    int applyBacklashCompensation(int steps);

    /**
     * Validate position
     */
    bool isValidPosition(uint8_t position) const;

    /**
     * Update system state after movement
     */
    void updatePositionTracking();

    /**
     * Handle movement timeout
     */
    void checkMovementTimeout();

    /**
     * Error handling
     */
    void setError(uint8_t errorCode);
};