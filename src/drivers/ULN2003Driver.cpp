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
    stepper.setSpeed(DEFAULT_SPEED);
    stepper.setMaxSpeed(DEFAULT_MAX_SPEED);
    stepper.setAcceleration(DEFAULT_ACCELERATION);

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
    long pos = stepper.currentPosition();
    return directionReversed ? -pos : pos;
}

long ULN2003Driver::getTargetPosition() const {
    long pos = stepper.targetPosition();
    return directionReversed ? -pos : pos;
}

bool ULN2003Driver::run() {
    if (!motorEnabled) {
        return false;
    }
    return stepper.run();
}

void ULN2003Driver::runToPosition() {
    if (!motorEnabled) {
        return;
    }
    stepper.runToPosition();
}

bool ULN2003Driver::isRunning() const {
    return motorEnabled && stepper.isRunning();
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
    return stepper.speed();
}

float ULN2003Driver::getMaxSpeed() const {
    return stepper.maxSpeed();
}

float ULN2003Driver::getAcceleration() const {
    return stepper.acceleration();
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