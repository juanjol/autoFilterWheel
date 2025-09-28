#pragma once

#include "MotorDriver.h"
#include <stdint.h>

#ifdef MOTOR_DRIVER_TMC2130
#include "../config.h"
#include <TMCStepper.h>
#include <AccelStepper.h>
#include <SPI.h>

/**
 * TMC2130 Driver implementation for high-performance stepper control
 * Supports SPI communication, microstepping, StallGuard, and StealthChop
 */
class TMC2130Driver : public MotorDriver {
private:
    // TMC2130 driver instance
    TMC2130Stepper* tmcDriver;
    AccelStepper* stepper;

    // Pin assignments
    uint8_t stepPin, dirPin, enablePin;
    uint8_t csPin;  // SPI chip select pin

    // Motor state
    bool motorEnabled;
    bool directionReversed;
    long currentPosition;
    long targetPosition;
    bool isMoving;

    // TMC2130 specific settings
    uint16_t microsteps;
    uint16_t currentMA;
    bool stealthChopEnabled;
    bool stallGuardEnabled;
    int8_t stallGuardThreshold;

    // Movement parameters
    float speed;
    float maxSpeed;
    float acceleration;

    // Helper methods for EEPROM
    void saveConfigToEEPROM();
    void loadConfigFromEEPROM();

public:
    /**
     * Constructor for TMC2130 driver
     * @param stepPin Step pulse pin
     * @param dirPin Direction control pin
     * @param enablePin Enable/disable pin
     * @param csPin SPI chip select pin
     */
    TMC2130Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin, uint8_t csPin);

    // MotorDriver interface implementation
    void init() override;
    void move(long steps) override;
    void moveTo(long position) override;
    void setCurrentPosition(long position) override;
    long getCurrentPosition() const override;
    long getTargetPosition() const override;

    bool run() override;
    void runToPosition() override;
    bool isRunning() const override;
    void stop() override;
    void emergencyStop() override;

    void setSpeed(float speed) override;
    void setMaxSpeed(float maxSpeed) override;
    void setAcceleration(float acceleration) override;
    float getSpeed() const override;
    float getMaxSpeed() const override;
    float getAcceleration() const override;

    void enableMotor() override;
    void disableMotor() override;
    bool isMotorEnabled() const override;

    void setDirectionReversed(bool reversed) override;
    bool isDirectionReversed() const override;

    // Advanced capabilities
    bool supportsMicrostepping() const override { return true; }
    bool supportsStallDetection() const override { return true; }
    bool supportsCoolStep() const override { return false; }  // TMC2130 doesn't have CoolStep

    const char* getDriverName() const override { return "TMC2130"; }
    const char* getDriverVersion() const override { return "1.0.0"; }

    // TMC2130-specific advanced features
    void setMicrosteps(uint16_t microsteps) override;
    uint16_t getMicrosteps() const override;
    void setCurrent(uint16_t currentMA) override;
    uint16_t getCurrent() const override;
    void setStealthChopEnabled(bool enabled) override;
    bool isStealthChopEnabled() const override;

    // TMC2130 unique features
    void setStallGuardEnabled(bool enabled);
    bool isStallGuardEnabled() const;
    void setStallGuardThreshold(int8_t threshold);
    int8_t getStallGuardThreshold() const;
    bool isStallDetected() const;

    // Diagnostics
    uint32_t getDriverStatus() const;
    float getSupplyVoltage() const;
    float getDriverTemperature() const;
    uint32_t getLoadValue() const;

    // TMC Status and Error Reporting
    String getTMCStatusString() const;
    bool checkTMCCommunication() const;

    // Sensorless homing capability
    bool performSensorlessHoming(bool direction, uint32_t timeoutMs = 10000);
};

#else // Not using TMC2130

// Placeholder class when TMC2130 is not selected
class TMC2130Driver : public MotorDriver {
public:
    TMC2130Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin, uint8_t csPin) {}

    void init() override {}
    void move(long steps) override {}
    void moveTo(long position) override {}
    void setCurrentPosition(long position) override {}
    long getCurrentPosition() const override { return 0; }
    long getTargetPosition() const override { return 0; }
    bool run() override { return false; }
    void runToPosition() override {}
    bool isRunning() const override { return false; }
    void stop() override {}
    void emergencyStop() override {}
    void setSpeed(float speed) override {}
    void setMaxSpeed(float maxSpeed) override {}
    void setAcceleration(float acceleration) override {}
    float getSpeed() const override { return 0; }
    float getMaxSpeed() const override { return 0; }
    float getAcceleration() const override { return 0; }
    void enableMotor() override {}
    void disableMotor() override {}
    bool isMotorEnabled() const override { return false; }
    void setDirectionReversed(bool reversed) override {}
    bool isDirectionReversed() const override { return false; }
    const char* getDriverName() const override { return "TMC2130_DISABLED"; }
    const char* getDriverVersion() const override { return "0.0.0"; }
    bool supportsMicrostepping() const override { return false; }
    bool supportsStallDetection() const override { return false; }
    bool supportsCoolStep() const override { return false; }
};

#endif // MOTOR_DRIVER_TMC2130