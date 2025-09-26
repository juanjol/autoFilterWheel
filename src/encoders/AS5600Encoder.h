#pragma once

#include "EncoderInterface.h"
#include <Wire.h>

/**
 * AS5600 Magnetic Encoder Implementation
 * 12-bit magnetic rotary position sensor
 */
class AS5600Encoder : public EncoderInterface {
private:
    static constexpr uint8_t AS5600_ADDRESS = 0x36;

    // AS5600 Register addresses
    static constexpr uint8_t AS5600_RAW_ANGLE_H = 0x0C;
    static constexpr uint8_t AS5600_RAW_ANGLE_L = 0x0D;
    static constexpr uint8_t AS5600_ANGLE_H = 0x0E;
    static constexpr uint8_t AS5600_ANGLE_L = 0x0F;
    static constexpr uint8_t AS5600_STATUS = 0x0B;
    static constexpr uint8_t AS5600_AGC = 0x1A;
    static constexpr uint8_t AS5600_MAGNITUDE_H = 0x1B;
    static constexpr uint8_t AS5600_MAGNITUDE_L = 0x1C;

    // Status register bits
    static constexpr uint8_t AS5600_STATUS_MH = 0x08;  // Magnet too strong
    static constexpr uint8_t AS5600_STATUS_ML = 0x10;  // Magnet too weak
    static constexpr uint8_t AS5600_STATUS_MD = 0x20;  // Magnet detected

    TwoWire* wire;
    float angleOffset;
    bool available;
    uint16_t lastRawValue;
    bool movementDetected;

    // Performance tracking
    uint32_t readCount;
    uint32_t errorCount;

    static constexpr uint16_t RESOLUTION = 4096;  // 12-bit resolution
    static constexpr float DEGREES_PER_COUNT = 360.0f / RESOLUTION;

public:
    /**
     * Constructor
     * @param wireInterface I2C interface to use
     */
    AS5600Encoder(TwoWire* wireInterface = &Wire);

    // EncoderInterface implementation
    bool init() override;
    bool isAvailable() const override;
    float getAngle() override;
    uint16_t getRawValue() override;
    void setAngleOffset(float offset) override;
    float getAngleOffset() const override;
    uint16_t getResolution() const override;
    const char* getEncoderType() const override;
    bool hasMovementDetected() override;
    void resetMovementDetection() override;
    bool isHealthy() const override;
    bool performSelfTest() override;

    // AS5600-specific methods

    /**
     * Get magnet status
     * @return 0=OK, 1=too weak, 2=too strong, 3=not detected
     */
    uint8_t getMagnetStatus();

    /**
     * Get automatic gain control value
     */
    uint8_t getAGC();

    /**
     * Get magnetic field magnitude
     */
    uint16_t getMagnitude();

    /**
     * Check if magnet is properly positioned
     */
    bool isMagnetPositionOK();

    /**
     * Get error statistics
     */
    struct ErrorStats {
        uint32_t totalReads;
        uint32_t errors;
        float errorRate;
    };

    ErrorStats getErrorStats();

    /**
     * Reset error statistics
     */
    void resetErrorStats();

private:
    /**
     * Read 16-bit value from AS5600 register
     */
    uint16_t readRegister16(uint8_t reg);

    /**
     * Read 8-bit value from AS5600 register
     */
    uint8_t readRegister8(uint8_t reg);

    /**
     * Check if AS5600 is responding
     */
    bool testConnection();

    /**
     * Normalize angle to 0-360 range
     */
    float normalizeAngle(float angle);
};