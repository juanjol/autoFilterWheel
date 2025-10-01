#pragma once

#include "MotorDriver.h"
#include <AccelStepper.h>

/**
 * ULN2003 Driver implementation for 28BYJ-48 stepper motor
 * Supports unipolar stepper motors with 4-wire control
 */
class ULN2003Driver : public MotorDriver {
private:
    AccelStepper stepper;
    bool motorEnabled;
    bool directionReversed;

    // Pin assignments
    uint8_t pin1, pin2, pin3, pin4;

    // Backlash compensation
    int backlashSteps;
    bool backlashEnabled;
    bool lastMoveWasForward;

    // Unidirectional mode settings
    bool unidirectionalMode;

    // Calibration state
    bool revolutionCalibrationActive;
    long revolutionCalibrationStartPos;
    bool backlashCalibrationActive;
    int backlashCalibrationSteps;

    // Configurable steps per revolution
    int stepsPerRevolution;

    // Motor specifications
    static constexpr uint16_t STEPS_PER_REVOLUTION = 2048;  // 28BYJ-48 with internal gearing
    static constexpr float DEFAULT_SPEED = 300.0;           // steps/second
    static constexpr float DEFAULT_MAX_SPEED = 500.0;       // steps/second
    static constexpr float DEFAULT_ACCELERATION = 1000.0;   // steps/second² (increased)

public:
    /**
     * Constructor for ULN2003 driver
     * @param p1, p2, p3, p4 GPIO pins connected to ULN2003 IN1-IN4
     */
    ULN2003Driver(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);

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

    // Capability reporting
    bool supportsMicrostepping() const override { return false; }
    bool supportsStallDetection() const override { return false; }
    bool supportsCoolStep() const override { return false; }

    const char* getDriverName() const override { return "ULN2003"; }
    const char* getDriverVersion() const override { return "1.0.0"; }

    // Backlash compensation overrides
    void setBacklashSteps(int steps) override;
    int getBacklashSteps() const override;
    void setBacklashEnabled(bool enabled) override;
    bool isBacklashEnabled() const override;

    // Steps per revolution overrides
    void setStepsPerRevolution(int steps) override;
    int getStepsPerRevolution() const override;

    // Calibration method overrides
    void startRevolutionCalibration() override;
    void adjustRevolutionCalibration(int steps) override;
    int finishRevolutionCalibration() override;
    void startBacklashCalibration() override;
    int backlashTestStep(int steps) override;
    bool markBacklashMovement() override;
    int getCurrentBacklashSteps() const override;
    int finishBacklashCalibration() override;

    // Unidirectional mode
    void setUnidirectionalMode(bool enabled);
    bool isUnidirectionalMode() const;

    // ULN2003-specific methods
    void forceAllPinsLow();  // Ensure complete motor shutdown
    void stepForward(long steps) override;
    void stepBackward(long steps) override;

private:
    // Internal backlash compensation logic
    void moveWithBacklashCompensation(long targetPosition);
    bool needsBacklashCompensation(long targetPosition) const;
};