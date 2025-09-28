#pragma once

#include "MotorDriver.h"
#include <stdint.h>

#ifdef MOTOR_DRIVER_TMC2209
#include "../config.h"
#include <TMCStepper.h>
#include <AccelStepper.h>

/**
 * TMC2209 Driver implementation for high-performance stepper control
 * Supports UART communication, microstepping, stall detection, and StealthChop
 * Compatible with TMC2208 in write-only mode
 */
class TMC2209Driver : public MotorDriver {
private:
    // TMC2209 driver instance
    TMC2209Stepper* tmcDriver;
    AccelStepper* stepper;
    HardwareSerial* tmcSerial;

    // Pin assignments
    uint8_t stepPin, dirPin, enablePin;
    uint8_t rxPin, txPin;  // UART pins
    uint8_t slaveAddress;

    // Motor state
    bool motorEnabled;
    bool directionReversed;
    long currentPosition;
    long targetPosition;
    bool isMoving;

    // TMC2209 specific settings
    uint16_t microsteps;
    uint16_t currentMA;
    bool stealthChopEnabled;
    bool coolStepEnabled;

    // Movement parameters
    float speed;
    float maxSpeed;
    float acceleration;

    // Helper methods for EEPROM
    void saveConfigToEEPROM();
    void loadConfigFromEEPROM();

public:
    /**
     * Constructor for TMC2209 driver
     * @param stepPin Step pulse pin
     * @param dirPin Direction control pin
     * @param enablePin Enable/disable pin
     * @param rxPin UART RX pin for communication
     * @param txPin UART TX pin for communication
     * @param slaveAddr TMC2209 slave address (0-3)
     */
    TMC2209Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                  uint8_t rxPin, uint8_t txPin, uint8_t slaveAddr = 0);

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
    bool supportsCoolStep() const override { return true; }

    const char* getDriverName() const override { return "TMC2209"; }
    const char* getDriverVersion() const override { return "1.0.0"; }

    // TMC2209-specific advanced features
    void setMicrosteps(uint16_t microsteps) override;
    uint16_t getMicrosteps() const override;
    void setCurrent(uint16_t currentMA) override;
    uint16_t getCurrent() const override;
    void setStealthChopEnabled(bool enabled) override;
    bool isStealthChopEnabled() const override;

    // TMC2209 unique features
    void setCoolStepEnabled(bool enabled);
    bool isCoolStepEnabled() const;
    void setStallThreshold(int8_t threshold);
    int8_t getStallThreshold() const;
    bool isStallDetected() const;

    // Diagnostics
    uint32_t getDriverStatus() const;
    float getSupplyVoltage() const;
    float getDriverTemperature() const;

    // TMC Status and Error Reporting
    String getTMCStatusString() const;
    bool checkTMCCommunication() const;
};

#else // Not using TMC2209

// Placeholder class when TMC2209 is not selected
class TMC2209Driver : public MotorDriver {
public:
    TMC2209Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                  uint8_t rxPin, uint8_t txPin, uint8_t slaveAddr = 0) {}

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
    const char* getDriverName() const override { return "TMC2209_DISABLED"; }
    const char* getDriverVersion() const override { return "0.0.0"; }
    bool supportsMicrostepping() const override { return false; }
    bool supportsStallDetection() const override { return false; }
    bool supportsCoolStep() const override { return false; }
};

#endif // MOTOR_DRIVER_TMC2209