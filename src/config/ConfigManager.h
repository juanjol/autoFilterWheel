#pragma once

#include <Arduino.h>

/**
 * Configuration Manager
 * Handles all EEPROM storage and configuration persistence
 */
class ConfigManager {
private:
    // EEPROM Layout (matches current implementation)
    static constexpr uint16_t EEPROM_SIZE = 512;

    // Calibration flags
    static constexpr uint16_t EEPROM_CALIBRATION_FLAG = 0x00;       // 4 bytes
    static constexpr uint16_t EEPROM_AS5600_ANGLE_OFFSET = 0x04;    // 4 bytes (float)
    static constexpr uint16_t EEPROM_CURRENT_POSITION = 0x08;       // 1 byte

    // Filter configuration
    static constexpr uint16_t EEPROM_FILTER_NAMES_FLAG = 0x0C;      // 4 bytes
    static constexpr uint16_t EEPROM_FILTER_COUNT = 0x10;           // 1 byte
    static constexpr uint16_t EEPROM_FILTER_NAMES_START = 0x20;     // 16 bytes per name

    // Revolution calibration
    static constexpr uint16_t EEPROM_REVOLUTION_FLAG = 0x100;       // 4 bytes
    static constexpr uint16_t EEPROM_STEPS_PER_REVOLUTION = 0x104;  // 2 bytes

    // Backlash calibration
    static constexpr uint16_t EEPROM_BACKLASH_FLAG = 0x108;         // 4 bytes
    static constexpr uint16_t EEPROM_BACKLASH_STEPS = 0x10C;        // 1 byte

    // Motor configuration
    static constexpr uint16_t EEPROM_MOTOR_CONFIG_FLAG = 0x110;     // 4 bytes
    static constexpr uint16_t EEPROM_MOTOR_SPEED = 0x114;           // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_MAX_SPEED = 0x116;       // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_ACCELERATION = 0x118;    // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_DISABLE_DELAY = 0x11A;   // 2 bytes

    // Direction configuration
    static constexpr uint16_t EEPROM_DIRECTION_FLAG = 0x124;        // 4 bytes
    static constexpr uint16_t EEPROM_DIRECTION_MODE = 0x128;        // 1 byte
    static constexpr uint16_t EEPROM_REVERSE_DIRECTION = 0x129;     // 1 byte

    // Magic bytes for validation
    static constexpr uint32_t CALIBRATION_MAGIC = 0xAA;
    static constexpr uint32_t FILTER_NAMES_MAGIC = 0xBB;
    static constexpr uint32_t REVOLUTION_MAGIC = 0xCC;
    static constexpr uint32_t BACKLASH_MAGIC = 0xDD;
    static constexpr uint32_t MOTOR_CONFIG_MAGIC = 0xEE;
    static constexpr uint32_t DIRECTION_CONFIG_MAGIC = 0xFF;

    // Configuration structures
    struct MotorConfig {
        uint16_t speed;
        uint16_t maxSpeed;
        uint16_t acceleration;
        uint16_t disableDelay;
    };

    struct DirectionConfig {
        uint8_t directionMode;
        bool reverseDirection;
    };

public:
    static constexpr uint8_t MAX_FILTER_COUNT = 8;
    static constexpr uint8_t MAX_FILTER_NAME_LENGTH = 15;

    /**
     * Initialize EEPROM and load configuration
     */
    void init();

    // ========================================
    // CALIBRATION PERSISTENCE
    // ========================================

    /**
     * Set system as calibrated
     */
    void setCalibrated(bool calibrated = true);

    /**
     * Check if system is calibrated
     */
    bool isCalibrated();

    /**
     * Save encoder angle offset
     */
    void saveAngleOffset(float angleOffset);

    /**
     * Load encoder angle offset
     */
    float loadAngleOffset();

    /**
     * Save current position
     */
    void saveCurrentPosition(uint8_t position);

    /**
     * Load current position
     */
    uint8_t loadCurrentPosition();

    // ========================================
    // FILTER CONFIGURATION
    // ========================================

    /**
     * Save filter count
     */
    void saveFilterCount(uint8_t count);

    /**
     * Load filter count
     */
    uint8_t loadFilterCount();

    /**
     * Save filter name
     */
    void saveFilterName(uint8_t filterIndex, const char* name);

    /**
     * Load filter name
     */
    String loadFilterName(uint8_t filterIndex);

    /**
     * Check if custom filter names are stored
     */
    bool hasCustomFilterNames();

    /**
     * Clear all filter names (reset to defaults)
     */
    void clearFilterNames();

    // ========================================
    // REVOLUTION CALIBRATION
    // ========================================

    /**
     * Save revolution calibration
     */
    void saveRevolutionCalibration(uint16_t stepsPerRevolution);

    /**
     * Load revolution calibration
     */
    uint16_t loadRevolutionCalibration();

    /**
     * Check if revolution calibration exists
     */
    bool hasRevolutionCalibration();

    /**
     * Clear revolution calibration
     */
    void clearRevolutionCalibration();

    // ========================================
    // BACKLASH CALIBRATION
    // ========================================

    /**
     * Save backlash calibration
     */
    void saveBacklashCalibration(uint8_t backlashSteps);

    /**
     * Load backlash calibration
     */
    uint8_t loadBacklashCalibration();

    /**
     * Check if backlash calibration exists
     */
    bool hasBacklashCalibration();

    /**
     * Clear backlash calibration
     */
    void clearBacklashCalibration();

    // ========================================
    // MOTOR CONFIGURATION
    // ========================================

    /**
     * Save motor configuration
     */
    void saveMotorConfig(uint16_t speed, uint16_t maxSpeed,
                        uint16_t acceleration, uint16_t disableDelay);

    /**
     * Load motor configuration
     */
    MotorConfig loadMotorConfig();

    /**
     * Check if motor configuration exists
     */
    bool hasMotorConfig();

    /**
     * Clear motor configuration (reset to defaults)
     */
    void clearMotorConfig();

    /**
     * Individual motor parameter save methods
     */
    void saveMotorSpeed(uint16_t speed);
    void saveMaxMotorSpeed(uint16_t maxSpeed);
    void saveMotorAcceleration(uint16_t acceleration);
    void saveMotorDisableDelay(uint16_t disableDelay);
    void resetMotorConfiguration();

    /**
     * Individual direction parameter save methods
     */
    void saveDirectionMode(bool bidirectional);
    void saveReverseDirection(bool reverse);

    /**
     * Save steps per revolution (for revolution calibration)
     */
    void saveStepsPerRevolution(uint16_t steps);

    // ========================================
    // DIRECTION CONFIGURATION
    // ========================================

    /**
     * Save direction configuration
     */
    void saveDirectionConfig(uint8_t directionMode, bool reverseDirection);

    /**
     * Load direction configuration
     */
    DirectionConfig loadDirectionConfig();

    /**
     * Check if direction configuration exists
     */
    bool hasDirectionConfig();

    /**
     * Clear direction configuration (reset to defaults)
     */
    void clearDirectionConfig();

    // ========================================
    // UTILITY METHODS
    // ========================================

    /**
     * Clear all EEPROM data (factory reset)
     */
    void factoryReset();

    /**
     * Get configuration summary
     */
    String getConfigSummary();

    /**
     * Validate EEPROM integrity
     */
    bool validateEEPROM();

    /**
     * Get EEPROM usage statistics
     */
    struct EEPROMStats {
        uint16_t totalSize;
        uint16_t usedSize;
        uint16_t freeSize;
        uint8_t numStoredConfigs;
    };

    EEPROMStats getEEPROMStats();

private:
    /**
     * EEPROM utility methods
     */
    void writeUint32(uint16_t address, uint32_t value);
    uint32_t readUint32(uint16_t address);
    void writeUint16(uint16_t address, uint16_t value);
    uint16_t readUint16(uint16_t address);
    void writeUint8(uint16_t address, uint8_t value);
    uint8_t readUint8(uint16_t address);
    void writeFloat(uint16_t address, float value);
    float readFloat(uint16_t address);
    void writeString(uint16_t address, const char* str, uint8_t maxLength);
    String readString(uint16_t address, uint8_t maxLength);
};