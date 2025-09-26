#pragma once

#include "MotorDriver.h"
#include "ULN2003Driver.h"
#include "TMC2209Driver.h"
#include <memory>

/**
 * Motor Driver Types
 */
enum class MotorDriverType {
    ULN2003_28BYJ48,    // ULN2003 with 28BYJ-48 stepper
    TMC2209_BIPOLAR,    // TMC2209 with bipolar stepper
    // Future drivers can be added here
    A4988_BIPOLAR,      // A4988 with bipolar stepper
    DRV8825_BIPOLAR     // DRV8825 with bipolar stepper
};

/**
 * Configuration structure for ULN2003 driver
 */
struct ULN2003Config {
    uint8_t pin1, pin2, pin3, pin4;
    float speed = 300.0;
    float maxSpeed = 500.0;
    float acceleration = 200.0;
    bool reverseDirection = false;
};

/**
 * Configuration structure for TMC2209 driver
 */
struct TMC2209Config {
    uint8_t stepPin, dirPin, enablePin;
    uint8_t rxPin, txPin;
    uint8_t slaveAddress = 0;
    uint16_t microsteps = 16;
    uint16_t currentMA = 800;
    float speed = 1000.0;
    float maxSpeed = 2000.0;
    float acceleration = 500.0;
    bool reverseDirection = false;
    bool stealthChopEnabled = true;
    bool coolStepEnabled = true;
};

/**
 * Factory class for creating motor driver instances
 */
class MotorDriverFactory {
public:
    /**
     * Create ULN2003 driver instance
     */
    static std::unique_ptr<MotorDriver> createULN2003Driver(const ULN2003Config& config);

    /**
     * Create TMC2209 driver instance
     */
    static std::unique_ptr<MotorDriver> createTMC2209Driver(const TMC2209Config& config);

    /**
     * Create driver by type with default configuration
     */
    static std::unique_ptr<MotorDriver> createDriver(MotorDriverType type);

    /**
     * Get driver type name as string
     */
    static const char* getDriverTypeName(MotorDriverType type);

    /**
     * Parse driver type from string
     */
    static MotorDriverType parseDriverType(const char* typeStr);
};