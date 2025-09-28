#include "MotorDriverFactory.h"
#include "../config.h"
#include <cstring>

// C++11 compatibility helper for make_unique
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_compat(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

std::unique_ptr<MotorDriver> MotorDriverFactory::createULN2003Driver(const ULN2003Config& config) {
    auto driver = make_unique_compat<ULN2003Driver>(config.pin1, config.pin2, config.pin3, config.pin4);

    driver->init();
    driver->setSpeed(config.speed);
    driver->setMaxSpeed(config.maxSpeed);
    driver->setAcceleration(config.acceleration);
    driver->setDirectionReversed(config.reverseDirection);

    return driver;
}

std::unique_ptr<MotorDriver> MotorDriverFactory::createTMC2209Driver(const TMC2209Config& config) {
    auto driver = make_unique_compat<TMC2209Driver>(
        config.stepPin, config.dirPin, config.enablePin,
        config.rxPin, config.txPin, config.slaveAddress
    );

    // TMC2209 driver is now fully implemented
    // Configuration will be applied in init()

    return driver;
}

std::unique_ptr<MotorDriver> MotorDriverFactory::createTMC2130Driver(const TMC2130Config& config) {
    auto driver = make_unique_compat<TMC2130Driver>(
        config.stepPin, config.dirPin, config.enablePin, config.csPin
    );

    // TMC2130 driver configuration will be applied in init()

    return driver;
}

std::unique_ptr<MotorDriver> MotorDriverFactory::createDriver(MotorDriverType type) {
    switch (type) {
        case MotorDriverType::ULN2003_28BYJ48: {
            ULN2003Config config;
            #ifdef MOTOR_DRIVER_ULN2003
                config.pin1 = MOTOR_PIN1;
                config.pin2 = MOTOR_PIN2;
                config.pin3 = MOTOR_PIN3;
                config.pin4 = MOTOR_PIN4;
            #else
                // Fallback defaults if not defined
                config.pin1 = 2;
                config.pin2 = 3;
                config.pin3 = 4;
                config.pin4 = 10;
            #endif
            return createULN2003Driver(config);
        }

        case MotorDriverType::TMC2209_BIPOLAR: {
            TMC2209Config config;
            #ifdef MOTOR_DRIVER_TMC2209
                config.stepPin = MOTOR_STEP_PIN;
                config.dirPin = MOTOR_DIR_PIN;
                config.enablePin = MOTOR_ENABLE_PIN;
                config.rxPin = MOTOR_RX_PIN;
                config.txPin = MOTOR_TX_PIN;
                config.microsteps = MOTOR_MICROSTEPS;
                config.currentMA = MOTOR_CURRENT_MA;
            #else
                // Fallback defaults if not defined
                config.stepPin = 2;
                config.dirPin = 3;
                config.enablePin = 4;
                config.rxPin = 7;
                config.txPin = 10;
                config.microsteps = 16;
                config.currentMA = 800;
            #endif
            return createTMC2209Driver(config);
        }

        case MotorDriverType::TMC2130_BIPOLAR: {
            TMC2130Config config;
            #ifdef MOTOR_DRIVER_TMC2130
                config.stepPin = MOTOR_STEP_PIN;
                config.dirPin = MOTOR_DIR_PIN;
                config.enablePin = MOTOR_ENABLE_PIN;
                config.csPin = TMC_CS_PIN;
                config.microsteps = MOTOR_MICROSTEPS;
                config.currentMA = MOTOR_CURRENT_MA;
                config.stealthChopEnabled = USE_STEALTHCHOP;
                config.stallGuardEnabled = USE_STALLGUARD;
                config.stallGuardThreshold = STALLGUARD_THRESHOLD;
            #else
                // Fallback defaults if not defined
                config.stepPin = 2;
                config.dirPin = 3;
                config.enablePin = 4;
                config.csPin = 10;
                config.microsteps = 16;
                config.currentMA = 800;
                config.stealthChopEnabled = true;
                config.stallGuardEnabled = true;
                config.stallGuardThreshold = 8;
            #endif
            return createTMC2130Driver(config);
        }

        default:
            return nullptr;
    }
}

const char* MotorDriverFactory::getDriverTypeName(MotorDriverType type) {
    switch (type) {
        case MotorDriverType::ULN2003_28BYJ48:
            return "ULN2003_28BYJ48";
        case MotorDriverType::TMC2209_BIPOLAR:
            return "TMC2209_BIPOLAR";
        case MotorDriverType::TMC2130_BIPOLAR:
            return "TMC2130_BIPOLAR";
        case MotorDriverType::A4988_BIPOLAR:
            return "A4988_BIPOLAR";
        case MotorDriverType::DRV8825_BIPOLAR:
            return "DRV8825_BIPOLAR";
        default:
            return "UNKNOWN";
    }
}

MotorDriverType MotorDriverFactory::parseDriverType(const char* typeStr) {
    if (strcmp(typeStr, "ULN2003_28BYJ48") == 0) {
        return MotorDriverType::ULN2003_28BYJ48;
    }
    if (strcmp(typeStr, "TMC2209_BIPOLAR") == 0) {
        return MotorDriverType::TMC2209_BIPOLAR;
    }
    if (strcmp(typeStr, "TMC2130_BIPOLAR") == 0) {
        return MotorDriverType::TMC2130_BIPOLAR;
    }
    if (strcmp(typeStr, "A4988_BIPOLAR") == 0) {
        return MotorDriverType::A4988_BIPOLAR;
    }
    if (strcmp(typeStr, "DRV8825_BIPOLAR") == 0) {
        return MotorDriverType::DRV8825_BIPOLAR;
    }

    // Default fallback
    return MotorDriverType::ULN2003_28BYJ48;
}