#include "ULN2003Driver.h"
#include <Arduino.h>

ULN2003Driver::ULN2003Driver(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)
    : stepper(AccelStepper::FULL4WIRE, p1, p3, p2, p4)  // AccelStepper pin order
    , motorEnabled(false)
    , directionReversed(false)
    , pin1(p1), pin2(p2), pin3(p3), pin4(p4)
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

    stepper.moveTo(position);
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