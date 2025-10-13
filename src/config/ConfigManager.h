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
    static constexpr uint16_t EEPROM_CUSTOM_ANGLES_FLAG = 0x11;     // 1 byte
    static constexpr uint16_t EEPROM_CUSTOM_ANGLES_START = 0x12;    // 4 bytes per angle (float) x 9 = 36 bytes
    static constexpr uint16_t EEPROM_FILTER_NAMES_START = 0x40;     // 16 bytes per name (moved to avoid overlap)

    // Motor configuration
    static constexpr uint16_t EEPROM_MOTOR_CONFIG_FLAG = 0x110;     // 4 bytes
    static constexpr uint16_t EEPROM_MOTOR_SPEED = 0x114;           // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_MAX_SPEED = 0x116;       // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_ACCELERATION = 0x118;    // 2 bytes
    static constexpr uint16_t EEPROM_MOTOR_DISABLE_DELAY = 0x11A;   // 2 bytes

    // Direction configuration (motor + encoder)
    static constexpr uint16_t EEPROM_DIRECTION_CONFIG_FLAG = 0x11C; // 4 bytes
    static constexpr uint16_t EEPROM_MOTOR_DIRECTION_INVERTED = 0x120; // 1 byte (bool)
    static constexpr uint16_t EEPROM_ENCODER_DIRECTION_INVERTED = 0x121; // 1 byte (bool)

    // Magic bytes for validation
    static constexpr uint32_t CALIBRATION_MAGIC = 0xAA;
    static constexpr uint32_t FILTER_NAMES_MAGIC = 0xBB;
    static constexpr uint32_t REVOLUTION_MAGIC = 0xCC;
    static constexpr uint32_t BACKLASH_MAGIC = 0xDD;
    static constexpr uint32_t MOTOR_CONFIG_MAGIC = 0xEE;
    static constexpr uint32_t DIRECTION_CONFIG_MAGIC = 0xFF;
    static constexpr uint8_t CUSTOM_ANGLES_MAGIC = 0xCA;  // Custom Angles magic byte

    // Configuration structures
    struct MotorConfig {
        uint16_t speed;
        uint16_t maxSpeed;
        uint16_t acceleration;
        uint16_t disableDelay;
    };

    struct DirectionConfig {
        bool motorDirectionInverted;
        bool encoderDirectionInverted;
    };

public:
    static constexpr uint8_t MAX_FILTER_COUNT = 9;
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
    // CUSTOM ANGLE CALIBRATION
    // ========================================

    /**
     * Save custom angle for a filter position
     * @param position Filter position (1-9)
     * @param angle Angle in degrees (0-360)
     */
    void saveCustomAngle(uint8_t position, float angle);

    /**
     * Load custom angle for a filter position
     * @param position Filter position (1-9)
     * @return Angle in degrees, or -1.0 if not set
     */
    float loadCustomAngle(uint8_t position);

    /**
     * Check if custom angles are configured
     */
    bool hasCustomAngles();

    /**
     * Clear all custom angles (reset to uniform distribution)
     */
    void clearCustomAngles();

    /**
     * Get all custom angles as an array
     * @param angles Output array (must be at least 9 floats)
     * @return true if custom angles exist, false otherwise
     */
    bool loadAllCustomAngles(float* angles);

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

    // ========================================
    // DIRECTION CONFIGURATION
    // ========================================

    /**
     * Save direction configuration (motor and encoder inversion)
     */
    void saveDirectionConfig(bool motorInverted, bool encoderInverted);

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

    /**
     * Individual direction parameter save methods
     */
    void saveMotorDirectionInverted(bool inverted);
    void saveEncoderDirectionInverted(bool inverted);

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