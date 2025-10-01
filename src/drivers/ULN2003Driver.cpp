#include "ULN2003Driver.h"
#include <Arduino.h>

ULN2003Driver::ULN2003Driver(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)
    : stepper(AccelStepper::FULL4WIRE, p1, p3, p2, p4)  // AccelStepper pin order
    , motorEnabled(false)
    , directionReversed(false)
    , pin1(p1), pin2(p2), pin3(p3), pin4(p4)
    , backlashSteps(0)
    , backlashEnabled(false)
    , lastMoveWasForward(true)
    , unidirectionalMode(true)  // Default to unidirectional for astronomy use
    , revolutionCalibrationActive(false)
    , revolutionCalibrationStartPos(0)
    , backlashCalibrationActive(false)
    , backlashCalibrationSteps(0)
    , stepsPerRevolution(STEPS_PER_REVOLUTION)
{
}

void ULN2003Driver::init() {
    // Configure pins as outputs
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);

    // Initialize stepper with default settings
    stepper.setMaxSpeed(DEFAULT_MAX_SPEED);
    stepper.setAcceleration(DEFAULT_ACCELERATION);
    // Note: setSpeed() should be called AFTER setMaxSpeed()
    stepper.setSpeed(DEFAULT_SPEED);

    // Start with motor disabled
    disableMotor();
}

void ULN2003Driver::move(long steps) {
    if (directionReversed) {
        steps = -steps;
    }
    stepper.move(steps);
    motorEnabled = true;
}

void ULN2003Driver::moveTo(long position) {
    if (directionReversed) {
        position = -position;
    }

    // Use backlash compensation if enabled
    if (backlashEnabled || unidirectionalMode) {
        moveWithBacklashCompensation(position);
    } else {
        stepper.moveTo(position);
    }

    motorEnabled = true;
}

void ULN2003Driver::setCurrentPosition(long position) {
    if (directionReversed) {
        position = -position;
    }
    stepper.setCurrentPosition(position);
}

long ULN2003Driver::getCurrentPosition() const {
    long pos = const_cast<AccelStepper&>(stepper).currentPosition();
    return directionReversed ? -pos : pos;
}

long ULN2003Driver::getTargetPosition() const {
    long pos = const_cast<AccelStepper&>(stepper).targetPosition();
    return directionReversed ? -pos : pos;
}

bool ULN2003Driver::run() {
    if (!motorEnabled) {
        return false;
    }

    // Simple approach: just run the stepper
    return stepper.run();
}

void ULN2003Driver::runToPosition() {
    if (!motorEnabled) {
        return;
    }
    stepper.runToPosition();
}

bool ULN2003Driver::isRunning() const {
    return motorEnabled && const_cast<AccelStepper&>(stepper).isRunning();
}

void ULN2003Driver::stop() {
    stepper.stop();
}

void ULN2003Driver::emergencyStop() {
    stepper.stop();
    forceAllPinsLow();
    motorEnabled = false;
}

void ULN2003Driver::setSpeed(float speed) {
    stepper.setSpeed(speed);
}

void ULN2003Driver::setMaxSpeed(float maxSpeed) {
    stepper.setMaxSpeed(maxSpeed);
}

void ULN2003Driver::setAcceleration(float acceleration) {
    stepper.setAcceleration(acceleration);
}

float ULN2003Driver::getSpeed() const {
    return const_cast<AccelStepper&>(stepper).speed();
}

float ULN2003Driver::getMaxSpeed() const {
    return const_cast<AccelStepper&>(stepper).maxSpeed();
}

float ULN2003Driver::getAcceleration() const {
    return const_cast<AccelStepper&>(stepper).acceleration();
}

void ULN2003Driver::enableMotor() {
    motorEnabled = true;
}

void ULN2003Driver::disableMotor() {
    motorEnabled = false;
    forceAllPinsLow();
}

bool ULN2003Driver::isMotorEnabled() const {
    return motorEnabled;
}

void ULN2003Driver::setDirectionReversed(bool reversed) {
    directionReversed = reversed;
}

bool ULN2003Driver::isDirectionReversed() const {
    return directionReversed;
}

void ULN2003Driver::forceAllPinsLow() {
    // Ensure all control pins are LOW to completely disable motor
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    digitalWrite(pin3, LOW);
    digitalWrite(pin4, LOW);
}

// ========================================
// BACKLASH COMPENSATION METHODS
// ========================================

void ULN2003Driver::setBacklashSteps(int steps) {
    backlashSteps = abs(steps);  // Always positive
}

int ULN2003Driver::getBacklashSteps() const {
    return backlashSteps;
}

void ULN2003Driver::setBacklashEnabled(bool enabled) {
    backlashEnabled = enabled;
}

bool ULN2003Driver::isBacklashEnabled() const {
    return backlashEnabled;
}

// ========================================
// STEPS PER REVOLUTION METHODS
// ========================================

void ULN2003Driver::setStepsPerRevolution(int steps) {
    if (steps > 0 && steps <= 8192) {
        stepsPerRevolution = steps;
    }
}

int ULN2003Driver::getStepsPerRevolution() const {
    return stepsPerRevolution;
}

// ========================================
// UNIDIRECTIONAL MODE METHODS
// ========================================

void ULN2003Driver::setUnidirectionalMode(bool enabled) {
    unidirectionalMode = enabled;
}

bool ULN2003Driver::isUnidirectionalMode() const {
    return unidirectionalMode;
}

// ========================================
// ENHANCED MOVEMENT WITH BACKLASH COMPENSATION
// ========================================

void ULN2003Driver::moveWithBacklashCompensation(long targetPosition) {
    long currentPos = getCurrentPosition();
    bool movingForward = targetPosition > currentPos;

    // In unidirectional mode, always approach target from the same direction
    if (unidirectionalMode) {
        // For unidirectional mode, we only move forward (clockwise)
        // If target is "behind" us, we go the long way around
        long directSteps = targetPosition - currentPos;
        long forwardSteps = directSteps;

        if (directSteps < 0) {
            // Target is behind us - go forward the long way
            forwardSteps = stepsPerRevolution + directSteps;
        }

        // Apply backlash compensation only if enabled and we're changing direction
        if (backlashEnabled && !lastMoveWasForward && forwardSteps > 0) {
            // First, move back by backlash amount, then forward to target + backlash
            stepper.move(-backlashSteps);
            while (stepper.isRunning()) {
                stepper.run();
            }
            forwardSteps += backlashSteps;
        }

        if (forwardSteps > 0) {
            stepper.move(forwardSteps);
            lastMoveWasForward = true;
        }
    } else {
        // Bidirectional mode - take shortest path
        if (backlashEnabled && needsBacklashCompensation(targetPosition)) {
            // Apply backlash compensation
            if (!movingForward && lastMoveWasForward) {
                // Moving backward after forward - add backlash
                stepper.move(-backlashSteps);
                while (stepper.isRunning()) {
                    stepper.run();
                }
            }
        }

        stepper.moveTo(targetPosition);
        lastMoveWasForward = movingForward;
    }
}

bool ULN2003Driver::needsBacklashCompensation(long targetPosition) const {
    if (!backlashEnabled || backlashSteps == 0) {
        return false;
    }

    long currentPos = getCurrentPosition();
    bool movingForward = targetPosition > currentPos;

    // Need compensation if changing direction from forward to backward
    return !movingForward && lastMoveWasForward;
}

// ========================================
// REVOLUTION CALIBRATION METHODS
// ========================================

void ULN2003Driver::startRevolutionCalibration() {
    revolutionCalibrationActive = true;
    revolutionCalibrationStartPos = getCurrentPosition();
}

void ULN2003Driver::adjustRevolutionCalibration(int steps) {
    if (!revolutionCalibrationActive) {
        return;
    }

    if (unidirectionalMode && steps < 0) {
        // In unidirectional mode, negative adjustment means go forward the long way
        steps = stepsPerRevolution + steps;
    }

    stepper.move(steps);
    lastMoveWasForward = steps > 0;
}

int ULN2003Driver::finishRevolutionCalibration() {
    if (!revolutionCalibrationActive) {
        return stepsPerRevolution;  // Default
    }

    revolutionCalibrationActive = false;
    long totalSteps = abs(getCurrentPosition() - revolutionCalibrationStartPos);

    // Ensure we have a reasonable value
    if (totalSteps < 1000 || totalSteps > 8192) {
        totalSteps = stepsPerRevolution;  // Fallback to current setting
    } else {
        // Update the stored steps per revolution with the calibrated value
        stepsPerRevolution = totalSteps;
    }

    return totalSteps;
}

// ========================================
// BACKLASH CALIBRATION METHODS
// ========================================

void ULN2003Driver::startBacklashCalibration() {
    backlashCalibrationActive = true;
    backlashCalibrationSteps = 0;
}

int ULN2003Driver::backlashTestStep(int steps) {
    if (!backlashCalibrationActive) {
        return 0;
    }

    // In unidirectional mode, we test backlash by moving forward, then trying to move back
    if (unidirectionalMode) {
        // Always move forward for consistency
        if (steps < 0) {
            steps = abs(steps);  // Convert to positive
        }
    }

    stepper.move(steps);
    backlashCalibrationSteps += abs(steps);
    lastMoveWasForward = steps > 0;

    return backlashCalibrationSteps;
}

bool ULN2003Driver::markBacklashMovement() {
    // Return the direction of the last movement
    return lastMoveWasForward;
}

int ULN2003Driver::getCurrentBacklashSteps() const {
    return backlashCalibrationSteps;
}

int ULN2003Driver::finishBacklashCalibration() {
    if (!backlashCalibrationActive) {
        return backlashSteps;  // Return current setting
    }

    backlashCalibrationActive = false;

    // Set the measured backlash as the compensation value
    backlashSteps = backlashCalibrationSteps;
    backlashEnabled = backlashSteps > 0;

    return backlashSteps;
}

// Override stepForward to handle manual stepping properly
void ULN2003Driver::stepForward(long steps) {
    // Enable motor first
    motorEnabled = true;

    // Reset position to 0 for each manual command
    stepper.setCurrentPosition(0);

    // Set target
    if (directionReversed) {
        stepper.moveTo(-steps);
    } else {
        stepper.moveTo(steps);
    }

    // Run to completion immediately (blocking)
    while (stepper.distanceToGo() != 0) {
        stepper.run();
        delay(1);  // Small delay to prevent watchdog issues
    }
}

// Override stepBackward to handle manual stepping properly
void ULN2003Driver::stepBackward(long steps) {
    // Enable motor first
    motorEnabled = true;

    // Reset position to 0 for each manual command
    stepper.setCurrentPosition(0);

    // Set target (negative for backward)
    if (directionReversed) {
        stepper.moveTo(steps);  // Reversed, so positive is backward
    } else {
        stepper.moveTo(-steps);
    }

    // Run to completion immediately (blocking)
    while (stepper.distanceToGo() != 0) {
        stepper.run();
        delay(1);  // Small delay to prevent watchdog issues
    }
}