#include "ConfigManager.h"
#include <EEPROM.h>

void ConfigManager::init() {
    EEPROM.begin(EEPROM_SIZE);
}

void ConfigManager::setCalibrated(bool calibrated) {
    if (calibrated) {
        writeUint32(EEPROM_CALIBRATION_FLAG, CALIBRATION_MAGIC);
    } else {
        writeUint32(EEPROM_CALIBRATION_FLAG, 0);
    }
}

bool ConfigManager::isCalibrated() {
    return readUint32(EEPROM_CALIBRATION_FLAG) == CALIBRATION_MAGIC;
}

void ConfigManager::saveAngleOffset(float angleOffset) {
    writeFloat(EEPROM_AS5600_ANGLE_OFFSET, angleOffset);
}

float ConfigManager::loadAngleOffset() {
    return readFloat(EEPROM_AS5600_ANGLE_OFFSET);
}

void ConfigManager::saveCurrentPosition(uint8_t position) {
    writeUint8(EEPROM_CURRENT_POSITION, position);
}

uint8_t ConfigManager::loadCurrentPosition() {
    uint8_t pos = readUint8(EEPROM_CURRENT_POSITION);
    return (pos >= 1 && pos <= MAX_FILTER_COUNT) ? pos : 1;
}

void ConfigManager::saveFilterCount(uint8_t count) {
    if (count >= 3 && count <= MAX_FILTER_COUNT) {
        writeUint8(EEPROM_FILTER_COUNT, count);
    }
}

uint8_t ConfigManager::loadFilterCount() {
    uint8_t count = readUint8(EEPROM_FILTER_COUNT);
    return (count >= 3 && count <= MAX_FILTER_COUNT) ? count : 5; // Default to 5
}

void ConfigManager::saveFilterName(uint8_t filterIndex, const char* name) {
    if (filterIndex >= 1 && filterIndex <= MAX_FILTER_COUNT) {
        // Mark that custom names are stored
        writeUint32(EEPROM_FILTER_NAMES_FLAG, FILTER_NAMES_MAGIC);

        // Calculate address for this filter name
        uint16_t address = EEPROM_FILTER_NAMES_START + ((filterIndex - 1) * (MAX_FILTER_NAME_LENGTH + 1));
        writeString(address, name, MAX_FILTER_NAME_LENGTH);
    }
}

String ConfigManager::loadFilterName(uint8_t filterIndex) {
    if (filterIndex < 1 || filterIndex > MAX_FILTER_COUNT) {
        return String("Filter ") + String(filterIndex);
    }

    if (!hasCustomFilterNames()) {
        // Return default names
        const char* defaultNames[] = {
            "Luminance", "Red", "Green", "Blue", "H-Alpha", "Filter 6", "Filter 7", "Filter 8"
        };
        return String(defaultNames[filterIndex - 1]);
    }

    uint16_t address = EEPROM_FILTER_NAMES_START + ((filterIndex - 1) * (MAX_FILTER_NAME_LENGTH + 1));
    return readString(address, MAX_FILTER_NAME_LENGTH);
}

bool ConfigManager::hasCustomFilterNames() {
    return readUint32(EEPROM_FILTER_NAMES_FLAG) == FILTER_NAMES_MAGIC;
}

void ConfigManager::clearFilterNames() {
    writeUint32(EEPROM_FILTER_NAMES_FLAG, 0);
}

void ConfigManager::saveRevolutionCalibration(uint16_t stepsPerRevolution) {
    writeUint32(EEPROM_REVOLUTION_FLAG, REVOLUTION_MAGIC);
    writeUint16(EEPROM_STEPS_PER_REVOLUTION, stepsPerRevolution);
}

uint16_t ConfigManager::loadRevolutionCalibration() {
    if (hasRevolutionCalibration()) {
        return readUint16(EEPROM_STEPS_PER_REVOLUTION);
    }
    return 2048; // Default for 28BYJ-48
}

bool ConfigManager::hasRevolutionCalibration() {
    return readUint32(EEPROM_REVOLUTION_FLAG) == REVOLUTION_MAGIC;
}

void ConfigManager::clearRevolutionCalibration() {
    writeUint32(EEPROM_REVOLUTION_FLAG, 0);
}

void ConfigManager::saveBacklashCalibration(uint8_t backlashSteps) {
    writeUint32(EEPROM_BACKLASH_FLAG, BACKLASH_MAGIC);
    writeUint8(EEPROM_BACKLASH_STEPS, backlashSteps);
}

uint8_t ConfigManager::loadBacklashCalibration() {
    if (hasBacklashCalibration()) {
        return readUint8(EEPROM_BACKLASH_STEPS);
    }
    return 0; // Default: no backlash compensation
}

bool ConfigManager::hasBacklashCalibration() {
    return readUint32(EEPROM_BACKLASH_FLAG) == BACKLASH_MAGIC;
}

void ConfigManager::clearBacklashCalibration() {
    writeUint32(EEPROM_BACKLASH_FLAG, 0);
}

void ConfigManager::saveMotorConfig(uint16_t speed, uint16_t maxSpeed,
                                   uint16_t acceleration, uint16_t disableDelay) {
    writeUint32(EEPROM_MOTOR_CONFIG_FLAG, MOTOR_CONFIG_MAGIC);
    writeUint16(EEPROM_MOTOR_SPEED, speed);
    writeUint16(EEPROM_MOTOR_MAX_SPEED, maxSpeed);
    writeUint16(EEPROM_MOTOR_ACCELERATION, acceleration);
    writeUint16(EEPROM_MOTOR_DISABLE_DELAY, disableDelay);
}

ConfigManager::MotorConfig ConfigManager::loadMotorConfig() {
    MotorConfig config;

    if (hasMotorConfig()) {
        config.speed = readUint16(EEPROM_MOTOR_SPEED);
        config.maxSpeed = readUint16(EEPROM_MOTOR_MAX_SPEED);
        config.acceleration = readUint16(EEPROM_MOTOR_ACCELERATION);
        config.disableDelay = readUint16(EEPROM_MOTOR_DISABLE_DELAY);
    } else {
        // Defaults
        config.speed = 300;
        config.maxSpeed = 500;
        config.acceleration = 200;
        config.disableDelay = 1000;
    }

    return config;
}

bool ConfigManager::hasMotorConfig() {
    return readUint32(EEPROM_MOTOR_CONFIG_FLAG) == MOTOR_CONFIG_MAGIC;
}

void ConfigManager::clearMotorConfig() {
    writeUint32(EEPROM_MOTOR_CONFIG_FLAG, 0);
}

void ConfigManager::saveDirectionConfig(uint8_t directionMode, bool reverseDirection) {
    writeUint32(EEPROM_DIRECTION_FLAG, DIRECTION_CONFIG_MAGIC);
    writeUint8(EEPROM_DIRECTION_MODE, directionMode);
    writeUint8(EEPROM_REVERSE_DIRECTION, reverseDirection ? 1 : 0);
}

ConfigManager::DirectionConfig ConfigManager::loadDirectionConfig() {
    DirectionConfig config;

    if (hasDirectionConfig()) {
        config.directionMode = readUint8(EEPROM_DIRECTION_MODE);
        config.reverseDirection = readUint8(EEPROM_REVERSE_DIRECTION) != 0;
    } else {
        // Defaults
        config.directionMode = 0; // Unidirectional
        config.reverseDirection = true; // Default reversed for 28BYJ-48
    }

    return config;
}

bool ConfigManager::hasDirectionConfig() {
    return readUint32(EEPROM_DIRECTION_FLAG) == DIRECTION_CONFIG_MAGIC;
}

void ConfigManager::clearDirectionConfig() {
    writeUint32(EEPROM_DIRECTION_FLAG, 0);
}

void ConfigManager::factoryReset() {
    // Clear all EEPROM data
    for (uint16_t i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0x00);
    }
    EEPROM.commit();
}

String ConfigManager::getConfigSummary() {
    String summary = "Configuration Summary:\n";
    summary += "Calibrated: " + String(isCalibrated() ? "YES" : "NO") + "\n";
    summary += "Filter Count: " + String(loadFilterCount()) + "\n";
    summary += "Custom Names: " + String(hasCustomFilterNames() ? "YES" : "NO") + "\n";
    summary += "Revolution Cal: " + String(hasRevolutionCalibration() ? "YES" : "NO") + "\n";
    summary += "Backlash Cal: " + String(hasBacklashCalibration() ? "YES" : "NO") + "\n";
    summary += "Motor Config: " + String(hasMotorConfig() ? "CUSTOM" : "DEFAULT") + "\n";
    summary += "Direction Config: " + String(hasDirectionConfig() ? "CUSTOM" : "DEFAULT");
    return summary;
}

bool ConfigManager::validateEEPROM() {
    // Basic validation - check if any magic bytes are corrupted
    bool valid = true;

    if (isCalibrated()) {
        valid &= (readUint32(EEPROM_CALIBRATION_FLAG) == CALIBRATION_MAGIC);
    }

    if (hasCustomFilterNames()) {
        valid &= (readUint32(EEPROM_FILTER_NAMES_FLAG) == FILTER_NAMES_MAGIC);
    }

    // Add more validation as needed
    return valid;
}

ConfigManager::EEPROMStats ConfigManager::getEEPROMStats() {
    EEPROMStats stats;
    stats.totalSize = EEPROM_SIZE;
    stats.usedSize = 0x12A; // Based on our current layout
    stats.freeSize = EEPROM_SIZE - stats.usedSize;

    stats.numStoredConfigs = 0;
    if (isCalibrated()) stats.numStoredConfigs++;
    if (hasCustomFilterNames()) stats.numStoredConfigs++;
    if (hasRevolutionCalibration()) stats.numStoredConfigs++;
    if (hasBacklashCalibration()) stats.numStoredConfigs++;
    if (hasMotorConfig()) stats.numStoredConfigs++;
    if (hasDirectionConfig()) stats.numStoredConfigs++;

    return stats;
}

// EEPROM utility methods
void ConfigManager::writeUint32(uint16_t address, uint32_t value) {
    EEPROM.write(address, (value >> 24) & 0xFF);
    EEPROM.write(address + 1, (value >> 16) & 0xFF);
    EEPROM.write(address + 2, (value >> 8) & 0xFF);
    EEPROM.write(address + 3, value & 0xFF);
    EEPROM.commit();
}

uint32_t ConfigManager::readUint32(uint16_t address) {
    uint32_t value = 0;
    value |= ((uint32_t)EEPROM.read(address)) << 24;
    value |= ((uint32_t)EEPROM.read(address + 1)) << 16;
    value |= ((uint32_t)EEPROM.read(address + 2)) << 8;
    value |= EEPROM.read(address + 3);
    return value;
}

void ConfigManager::writeUint16(uint16_t address, uint16_t value) {
    EEPROM.write(address, (value >> 8) & 0xFF);
    EEPROM.write(address + 1, value & 0xFF);
    EEPROM.commit();
}

uint16_t ConfigManager::readUint16(uint16_t address) {
    uint16_t value = 0;
    value |= ((uint16_t)EEPROM.read(address)) << 8;
    value |= EEPROM.read(address + 1);
    return value;
}

void ConfigManager::writeUint8(uint16_t address, uint8_t value) {
    EEPROM.write(address, value);
    EEPROM.commit();
}

uint8_t ConfigManager::readUint8(uint16_t address) {
    return EEPROM.read(address);
}

void ConfigManager::writeFloat(uint16_t address, float value) {
    union {
        float f;
        uint32_t i;
    } converter;
    converter.f = value;
    writeUint32(address, converter.i);
}

float ConfigManager::readFloat(uint16_t address) {
    union {
        float f;
        uint32_t i;
    } converter;
    converter.i = readUint32(address);
    return converter.f;
}

void ConfigManager::writeString(uint16_t address, const char* str, uint8_t maxLength) {
    uint8_t len = strlen(str);
    if (len > maxLength) len = maxLength;

    for (uint8_t i = 0; i < len; i++) {
        EEPROM.write(address + i, str[i]);
    }

    // Null terminate
    EEPROM.write(address + len, 0);

    EEPROM.commit();
}

String ConfigManager::readString(uint16_t address, uint8_t maxLength) {
    String result = "";

    for (uint8_t i = 0; i < maxLength; i++) {
        uint8_t c = EEPROM.read(address + i);
        if (c == 0) break; // Null terminator
        result += (char)c;
    }

    return result;
}

// Individual motor parameter save methods
void ConfigManager::saveMotorSpeed(uint16_t speed) {
    MotorConfig config = loadMotorConfig();
    config.speed = speed;
    saveMotorConfig(config.speed, config.maxSpeed, config.acceleration, config.disableDelay);
}

void ConfigManager::saveMaxMotorSpeed(uint16_t maxSpeed) {
    MotorConfig config = loadMotorConfig();
    config.maxSpeed = maxSpeed;
    saveMotorConfig(config.speed, config.maxSpeed, config.acceleration, config.disableDelay);
}

void ConfigManager::saveMotorAcceleration(uint16_t acceleration) {
    MotorConfig config = loadMotorConfig();
    config.acceleration = acceleration;
    saveMotorConfig(config.speed, config.maxSpeed, config.acceleration, config.disableDelay);
}

void ConfigManager::saveMotorDisableDelay(uint16_t disableDelay) {
    MotorConfig config = loadMotorConfig();
    config.disableDelay = disableDelay;
    saveMotorConfig(config.speed, config.maxSpeed, config.acceleration, config.disableDelay);
}

void ConfigManager::resetMotorConfiguration() {
    clearMotorConfig();
}

// Individual direction parameter save methods
void ConfigManager::saveDirectionMode(bool bidirectional) {
    DirectionConfig config = loadDirectionConfig();
    config.directionMode = bidirectional ? 1 : 0;
    saveDirectionConfig(config.directionMode, config.reverseDirection);
}

void ConfigManager::saveReverseDirection(bool reverse) {
    DirectionConfig config = loadDirectionConfig();
    config.reverseDirection = reverse;
    saveDirectionConfig(config.directionMode, config.reverseDirection);
}

// Save steps per revolution (for revolution calibration)
void ConfigManager::saveStepsPerRevolution(uint16_t steps) {
    saveRevolutionCalibration(steps);
}

// Save backlash steps
void ConfigManager::saveBacklashSteps(uint8_t steps) {
    saveBacklashCalibration(steps);
}