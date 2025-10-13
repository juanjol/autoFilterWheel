#include "AS5600Encoder.h"
#include "../config.h"
#include <Arduino.h>

AS5600Encoder::AS5600Encoder(TwoWire* wireInterface)
    : wire(wireInterface)
    , angleOffset(0.0f)
    , available(false)
    , lastRawValue(0)
    , movementDetected(false)
    , directionInverted(false)
    , previousAngle(0)
    , rotationDirection(0)
    , readCount(0)
    , errorCount(0)
{
}

bool AS5600Encoder::init() {
    if (!wire) {
        return false;
    }

    wire->begin();

    // Test connection to AS5600
    available = testConnection();

    if (available) {
        lastRawValue = getRawValue();
        previousAngle = lastRawValue;
        resetErrorStats();
    }

    return available;
}

bool AS5600Encoder::isAvailable() const {
    return available;
}

float AS5600Encoder::getAngle() {
    uint16_t rawValue = getRawValue();
    if (rawValue == 0xFFFF) {  // Error reading
        return -1.0f;
    }

    float angle = rawValue * DEGREES_PER_COUNT;

    // Invert encoder direction if configured (compile-time)
    #ifdef AS5600_INVERT_DIRECTION
    #if AS5600_INVERT_DIRECTION
    angle = 360.0f - angle;
    #endif
    #endif

    // Invert encoder direction if configured (runtime)
    if (directionInverted) {
        angle = 360.0f - angle;
    }

    angle = normalizeAngle(angle - angleOffset);

    // Check for movement and update direction
    int16_t delta = (int16_t)rawValue - (int16_t)previousAngle;

    // Handle wraparound (360°→0° or 0°→360°)
    if (delta > 2048) {
        delta -= 4096;  // Wraparound 360°→0° (going CCW)
    } else if (delta < -2048) {
        delta += 4096;  // Wraparound 0°→360° (going CW)
    }

    // Update direction based on delta
    const int16_t MOVEMENT_THRESHOLD = 5;  // Minimum counts to consider as movement
    if (abs(delta) > MOVEMENT_THRESHOLD) {
        movementDetected = true;
        rotationDirection = (delta > 0) ? 1 : -1;  // 1 = CW, -1 = CCW
        previousAngle = rawValue;
    } else {
        rotationDirection = 0;  // No significant movement
    }

    lastRawValue = rawValue;

    return angle;
}

uint16_t AS5600Encoder::getRawValue() {
    readCount++;

    uint16_t rawAngle = readRegister16(AS5600_RAW_ANGLE_H);

    if (rawAngle == 0xFFFF) {
        errorCount++;
    }

    return rawAngle;
}

void AS5600Encoder::setAngleOffset(float offset) {
    angleOffset = normalizeAngle(offset);
}

float AS5600Encoder::getAngleOffset() const {
    return angleOffset;
}

uint16_t AS5600Encoder::getResolution() const {
    return RESOLUTION;
}

const char* AS5600Encoder::getEncoderType() const {
    return "AS5600";
}

bool AS5600Encoder::hasMovementDetected() {
    return movementDetected;
}

void AS5600Encoder::resetMovementDetection() {
    movementDetected = false;
}

bool AS5600Encoder::isHealthy() const {
    if (!available) {
        return false;
    }

    // Check error rate
    if (readCount > 10 && (float)errorCount / readCount > 0.1f) {
        return false;  // More than 10% error rate
    }

    // Check magnet position
    return isMagnetPositionOK();
}

bool AS5600Encoder::performSelfTest() {
    if (!available) {
        return false;
    }

    // Test 1: Read angle multiple times and check for reasonable values
    uint16_t readings[5];
    for (int i = 0; i < 5; i++) {
        readings[i] = getRawValue();
        if (readings[i] == 0xFFFF) {
            return false;
        }
        delay(10);
    }

    // Test 2: Check that readings are stable (within reasonable range)
    uint16_t minVal = readings[0];
    uint16_t maxVal = readings[0];
    for (int i = 1; i < 5; i++) {
        if (readings[i] < minVal) minVal = readings[i];
        if (readings[i] > maxVal) maxVal = readings[i];
    }

    // Should not vary by more than 50 counts in stable conditions
    if (maxVal - minVal > 50) {
        return false;
    }

    // Test 3: Check magnet status
    if (!isMagnetPositionOK()) {
        return false;
    }

    return true;
}

uint8_t AS5600Encoder::getMagnetStatus() const {
    uint8_t status = readRegister8(AS5600_STATUS);

    if (!(status & AS5600_STATUS_MD)) {
        return 3;  // No magnet detected
    }
    if (status & AS5600_STATUS_ML) {
        return 1;  // Magnet too weak
    }
    if (status & AS5600_STATUS_MH) {
        return 2;  // Magnet too strong
    }

    return 0;  // Magnet OK
}

uint8_t AS5600Encoder::getAGC() {
    return readRegister8(AS5600_AGC);
}

uint16_t AS5600Encoder::getMagnitude() {
    return readRegister16(AS5600_MAGNITUDE_H);
}

bool AS5600Encoder::isMagnetPositionOK() const {
    return getMagnetStatus() == 0;
}

AS5600Encoder::ErrorStats AS5600Encoder::getErrorStats() {
    ErrorStats stats;
    stats.totalReads = readCount;
    stats.errors = errorCount;
    stats.errorRate = readCount > 0 ? (float)errorCount / readCount : 0.0f;
    return stats;
}

void AS5600Encoder::resetErrorStats() {
    readCount = 0;
    errorCount = 0;
}

uint16_t AS5600Encoder::readRegister16(uint8_t reg) const {
    wire->beginTransmission(AS5600_ADDRESS);
    wire->write(reg);
    if (wire->endTransmission() != 0) {
        return 0xFFFF;  // Error
    }

    wire->requestFrom(AS5600_ADDRESS, (uint8_t)2);
    if (wire->available() == 2) {
        uint16_t value = wire->read() << 8;
        value |= wire->read();
        return value & 0x0FFF;  // AS5600 is 12-bit
    }

    return 0xFFFF;  // Error
}

uint8_t AS5600Encoder::readRegister8(uint8_t reg) const {
    wire->beginTransmission(AS5600_ADDRESS);
    wire->write(reg);
    if (wire->endTransmission() != 0) {
        return 0xFF;  // Error
    }

    wire->requestFrom(AS5600_ADDRESS, (uint8_t)1);
    if (wire->available() == 1) {
        return wire->read();
    }

    return 0xFF;  // Error
}

bool AS5600Encoder::testConnection() {
    // Try to read the status register
    uint8_t status = readRegister8(AS5600_STATUS);
    return status != 0xFF;  // 0xFF indicates communication error
}

float AS5600Encoder::normalizeAngle(float angle) {
    while (angle < 0.0f) {
        angle += 360.0f;
    }
    while (angle >= 360.0f) {
        angle -= 360.0f;
    }
    return angle;
}

int8_t AS5600Encoder::getRotationDirection() {
    return rotationDirection;
}

int8_t AS5600Encoder::getExpectedDirection(float targetAngle) {
    // Get current angle
    float currentAngle = getAngle();
    if (currentAngle < 0) {
        return 0;  // Error reading angle
    }

    // Calculate the difference
    float diff = targetAngle - currentAngle;

    // Normalize difference to -180 to +180 range
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;

    // Determine expected direction based on shortest path
    if (abs(diff) < 5.0f) {
        return 0;  // Already at target
    }

    return (diff > 0) ? 1 : -1;  // 1 = CW, -1 = CCW
}

void AS5600Encoder::setDirectionInverted(bool inverted) {
    directionInverted = inverted;
}

bool AS5600Encoder::isDirectionInverted() const {
    return directionInverted;
}