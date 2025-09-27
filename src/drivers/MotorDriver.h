#pragma once

#include <Arduino.h>
#include <stdint.h>

/**
 * Abstract base class for motor drivers
 * Provides common interface for different stepper motor drivers
 */
class MotorDriver {
public:
    virtual ~MotorDriver() = default;

    // Basic motor control
    virtual void init() = 0;
    virtual void move(long steps) = 0;
    virtual void moveTo(long position) = 0;
    virtual void setCurrentPosition(long position) = 0;
    virtual long getCurrentPosition() const = 0;
    virtual long getTargetPosition() const = 0;

    // Movement execution
    virtual bool run() = 0;
    virtual void runToPosition() = 0;
    virtual bool isRunning() const = 0;
    virtual void stop() = 0;
    virtual void emergencyStop() = 0;

    // Motor configuration
    virtual void setSpeed(float speed) = 0;
    virtual void setMaxSpeed(float maxSpeed) = 0;
    virtual void setAcceleration(float acceleration) = 0;
    virtual float getSpeed() const = 0;
    virtual float getMaxSpeed() const = 0;
    virtual float getAcceleration() const = 0;

    // Power management
    virtual void enableMotor() = 0;
    virtual void disableMotor() = 0;
    virtual bool isMotorEnabled() const = 0;

    // Direction control
    virtual void setDirectionReversed(bool reversed) = 0;
    virtual bool isDirectionReversed() const = 0;

    // Driver-specific capabilities
    virtual bool supportsMicrostepping() const = 0;
    virtual bool supportsStallDetection() const = 0;
    virtual bool supportsCoolStep() const = 0;

    // Driver identification
    virtual const char* getDriverName() const = 0;
    virtual const char* getDriverVersion() const = 0;

    // Advanced features (optional, default implementations provided)
    virtual void setMicrosteps(uint16_t microsteps) { /* Default: no-op */ }
    virtual uint16_t getMicrosteps() const { return 1; }
    virtual void setCurrent(uint16_t currentMA) { /* Default: no-op */ }
    virtual uint16_t getCurrent() const { return 0; }
    virtual void setStealthChopEnabled(bool enabled) { /* Default: no-op */ }
    virtual bool isStealthChopEnabled() const { return false; }

    // Additional methods needed by command handlers
    virtual float getCurrentSpeed() const { return getSpeed(); }
    virtual void setDisableDelay(uint32_t delayMs) { /* Default: no-op */ }
    virtual uint32_t getDisableDelay() const { return 1000; }
    virtual void resetToDefaults() { /* Default: no-op */ }
    virtual void setDirectionMode(bool bidirectional) { /* Default: no-op */ }
    virtual bool getDirectionMode() const { return false; }
    virtual void setReverseDirection(bool reverse) { setDirectionReversed(reverse); }
    virtual bool getReverseDirection() const { return isDirectionReversed(); }
    virtual void stepForward(long steps) { move(steps); }
    virtual void stepBackward(long steps) { move(-steps); }
};