#pragma once

/**
 * Abstract interface for position encoders
 * Provides common interface for different encoder types
 */
class EncoderInterface {
public:
    virtual ~EncoderInterface() = default;

    /**
     * Initialize the encoder
     * @return true if initialization successful
     */
    virtual bool init() = 0;

    /**
     * Check if encoder is available and responding
     */
    virtual bool isAvailable() const = 0;

    /**
     * Get current angle in degrees (0-360)
     */
    virtual float getAngle() = 0;

    /**
     * Get raw encoder value
     */
    virtual uint16_t getRawValue() = 0;

    /**
     * Set angle offset for calibration
     */
    virtual void setAngleOffset(float offset) = 0;

    /**
     * Get current angle offset
     */
    virtual float getAngleOffset() const = 0;

    /**
     * Get encoder resolution (steps per revolution)
     */
    virtual uint16_t getResolution() const = 0;

    /**
     * Get encoder name/type
     */
    virtual const char* getEncoderType() const = 0;

    /**
     * Check if encoder detected movement since last call
     */
    virtual bool hasMovementDetected() = 0;

    /**
     * Reset movement detection
     */
    virtual void resetMovementDetection() = 0;

    /**
     * Get encoder health/status
     */
    virtual bool isHealthy() const = 0;

    /**
     * Perform encoder self-test
     */
    virtual bool performSelfTest() = 0;
};