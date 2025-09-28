#include "TMC2209Driver.h"
#include "../config.h"
#include <Arduino.h>
#include <EEPROM.h>

#ifdef MOTOR_DRIVER_TMC2209

// Constructor
TMC2209Driver::TMC2209Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                             uint8_t rxPin, uint8_t txPin, uint8_t slaveAddr)
    : stepPin(stepPin), dirPin(dirPin), enablePin(enablePin)
    , rxPin(rxPin), txPin(txPin), slaveAddress(slaveAddr)
    , tmcDriver(nullptr), stepper(nullptr), tmcSerial(nullptr)
    , motorEnabled(false), directionReversed(false)
    , currentPosition(0), targetPosition(0), isMoving(false)
    , microsteps(DEFAULT_MICROSTEPS), currentMA(DEFAULT_MOTOR_CURRENT)
    , stealthChopEnabled(USE_STEALTHCHOP), coolStepEnabled(USE_COOLSTEP)
    , speed(MOTOR_SPEED), maxSpeed(MAX_MOTOR_SPEED), acceleration(MOTOR_ACCELERATION)
{
}

// Initialize the TMC2209 driver
void TMC2209Driver::init() {
    // Configure pins
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, HIGH); // Disable motor initially (active low)

    // Initialize UART for TMC2209
    tmcSerial = &Serial1;
    tmcSerial->begin(TMC_SERIAL_BAUD, SERIAL_8N1, rxPin, txPin);

    // Create TMC2209 driver instance
    tmcDriver = new TMC2209Stepper(tmcSerial, TMC_R_SENSE, slaveAddress);

    // Initialize TMC2209
    tmcDriver->begin();

    // Test communication
    tmcDriver->toff(5); // Enable driver
    uint8_t test = tmcDriver->toff();
    if (test != 5) {
        Serial.println("ERROR: TMC2209 communication failed!");
        Serial.println("Check UART connections and power supply.");
        return;
    }

    // Load configuration from EEPROM if available
    loadConfigFromEEPROM();

    // Apply configuration
    tmcDriver->rms_current(currentMA);
    tmcDriver->microsteps(microsteps);

    // Configure StealthChop or SpreadCycle
    if (stealthChopEnabled) {
        tmcDriver->en_spreadCycle(false);
        tmcDriver->pwm_autoscale(true);
        tmcDriver->pwm_autograd(true);
        tmcDriver->TPWMTHRS(STEALTHCHOP_THRESHOLD);  // Use uppercase function name
    } else {
        tmcDriver->en_spreadCycle(true);
    }

    // Set hold current multiplier
    tmcDriver->hold_multiplier(MOTOR_HOLD_MULTIPLIER);

    // Configure stallGuard
    tmcDriver->TCOOLTHRS(0xFFFFF); // Enable stallGuard for all speeds
    tmcDriver->semin(5);
    tmcDriver->semax(2);
    tmcDriver->sedn(0b01);
    tmcDriver->SGTHRS(64);  // StallGuard threshold

    // Configure CoolStep if enabled
    if (coolStepEnabled) {
        tmcDriver->semin(1);
        tmcDriver->semax(0);
        tmcDriver->seup(1);
        tmcDriver->sedn(0);
    }

    // Initialize AccelStepper for movement control
    stepper = new AccelStepper(AccelStepper::DRIVER, stepPin, dirPin);
    stepper->setMaxSpeed(maxSpeed * microsteps);
    stepper->setAcceleration(acceleration * microsteps);
    stepper->setSpeed(speed * microsteps);

    Serial.println("TMC2209 initialized successfully");
    Serial.print("  Current: "); Serial.print(currentMA); Serial.println(" mA");
    Serial.print("  Microsteps: "); Serial.println(microsteps);
    Serial.print("  Mode: "); Serial.println(stealthChopEnabled ? "StealthChop" : "SpreadCycle");
}

// Movement methods
void TMC2209Driver::move(long steps) {
    if (!tmcDriver || !stepper) return;

    long actualSteps = steps * microsteps;
    stepper->move(actualSteps);
    targetPosition = currentPosition + steps;
    isMoving = true;
    enableMotor();
}

void TMC2209Driver::moveTo(long position) {
    if (!tmcDriver || !stepper) return;

    long actualPosition = position * microsteps;
    stepper->moveTo(actualPosition);
    targetPosition = position;
    isMoving = true;
    enableMotor();
}

void TMC2209Driver::setCurrentPosition(long position) {
    if (!stepper) return;

    currentPosition = position;
    targetPosition = position;
    stepper->setCurrentPosition(position * microsteps);
    isMoving = false;
}

long TMC2209Driver::getCurrentPosition() const {
    if (!stepper) return 0;
    return stepper->currentPosition() / microsteps;
}

long TMC2209Driver::getTargetPosition() const {
    return targetPosition;
}

bool TMC2209Driver::run() {
    if (!stepper || !motorEnabled) return false;

    bool stillRunning = stepper->run();

    if (stillRunning) {
        // Update current position
        currentPosition = stepper->currentPosition() / microsteps;

        // Check for stall detection
        if (tmcDriver && tmcDriver->diag()) {
            Serial.println("WARNING: Stall detected!");
            emergencyStop();
            return false;
        }
    } else {
        // Movement completed
        isMoving = false;
        currentPosition = targetPosition;

        // Optionally disable motor after movement
        if (AUTO_DISABLE_MOTOR) {
            // Schedule motor disable after delay
            // This should be handled by the controller
        }
    }

    return stillRunning;
}

void TMC2209Driver::runToPosition() {
    if (!stepper) return;

    enableMotor();
    while (stepper->distanceToGo() != 0) {
        stepper->run();

        // Check for stall
        if (tmcDriver && tmcDriver->diag()) {
            Serial.println("ERROR: Stall detected during movement!");
            emergencyStop();
            break;
        }
    }
    currentPosition = targetPosition;
    isMoving = false;
}

bool TMC2209Driver::isRunning() const {
    if (!stepper) return false;
    return stepper->isRunning();
}

void TMC2209Driver::stop() {
    if (!stepper) return;
    stepper->stop();
    isMoving = false;
}

void TMC2209Driver::emergencyStop() {
    if (stepper) {
        stepper->stop();
        stepper->setCurrentPosition(stepper->currentPosition());
    }
    isMoving = false;
    disableMotor();
}

// Speed and acceleration methods
void TMC2209Driver::setSpeed(float speed) {
    this->speed = speed;
    if (stepper) {
        stepper->setSpeed(speed * microsteps);
    }
}

void TMC2209Driver::setMaxSpeed(float maxSpeed) {
    this->maxSpeed = maxSpeed;
    if (stepper) {
        stepper->setMaxSpeed(maxSpeed * microsteps);
    }
}

void TMC2209Driver::setAcceleration(float acceleration) {
    this->acceleration = acceleration;
    if (stepper) {
        stepper->setAcceleration(acceleration * microsteps);
    }
}

float TMC2209Driver::getSpeed() const {
    return speed;
}

float TMC2209Driver::getMaxSpeed() const {
    return maxSpeed;
}

float TMC2209Driver::getAcceleration() const {
    return acceleration;
}

// Motor enable/disable
void TMC2209Driver::enableMotor() {
    motorEnabled = true;
    digitalWrite(enablePin, LOW); // Active low
}

void TMC2209Driver::disableMotor() {
    motorEnabled = false;
    digitalWrite(enablePin, HIGH); // Disable
}

bool TMC2209Driver::isMotorEnabled() const {
    return motorEnabled;
}

// Direction control
void TMC2209Driver::setDirectionReversed(bool reversed) {
    directionReversed = reversed;
    if (stepper) {
        stepper->setPinsInverted(reversed, false, false);
    }
}

bool TMC2209Driver::isDirectionReversed() const {
    return directionReversed;
}

// TMC2209-specific configuration
void TMC2209Driver::setMicrosteps(uint16_t microsteps) {
    // Validate microsteps value
    if (microsteps != 1 && microsteps != 2 && microsteps != 4 && microsteps != 8 &&
        microsteps != 16 && microsteps != 32 && microsteps != 64 &&
        microsteps != 128 && microsteps != 256) {
        Serial.print("Invalid microsteps value: "); Serial.println(microsteps);
        return;
    }

    this->microsteps = microsteps;
    if (tmcDriver) {
        tmcDriver->microsteps(microsteps);
    }

    // Update stepper speeds for new microstepping
    if (stepper) {
        stepper->setMaxSpeed(maxSpeed * microsteps);
        stepper->setAcceleration(acceleration * microsteps);
        stepper->setSpeed(speed * microsteps);
    }

    saveConfigToEEPROM();
    Serial.print("Microsteps set to: "); Serial.println(microsteps);
}

uint16_t TMC2209Driver::getMicrosteps() const {
    return microsteps;
}

void TMC2209Driver::setCurrent(uint16_t currentMA) {
    if (currentMA < MIN_MOTOR_CURRENT || currentMA > MAX_MOTOR_CURRENT) {
        Serial.print("Current out of range (");
        Serial.print(MIN_MOTOR_CURRENT);
        Serial.print("-");
        Serial.print(MAX_MOTOR_CURRENT);
        Serial.println(" mA)");
        return;
    }

    this->currentMA = currentMA;
    if (tmcDriver) {
        tmcDriver->rms_current(currentMA);
    }

    saveConfigToEEPROM();
    Serial.print("Motor current set to: "); Serial.print(currentMA); Serial.println(" mA");
}

uint16_t TMC2209Driver::getCurrent() const {
    return currentMA;
}

void TMC2209Driver::setStealthChopEnabled(bool enabled) {
    stealthChopEnabled = enabled;

    if (tmcDriver) {
        if (enabled) {
            tmcDriver->en_spreadCycle(false);
            tmcDriver->pwm_autoscale(true);
            tmcDriver->pwm_autograd(true);
            tmcDriver->TPWMTHRS(STEALTHCHOP_THRESHOLD);
        } else {
            tmcDriver->en_spreadCycle(true);
        }
    }

    saveConfigToEEPROM();
    Serial.print("StealthChop: "); Serial.println(enabled ? "Enabled" : "Disabled");
}

bool TMC2209Driver::isStealthChopEnabled() const {
    return stealthChopEnabled;
}

// TMC2209 unique features
void TMC2209Driver::setCoolStepEnabled(bool enabled) {
    coolStepEnabled = enabled;

    if (tmcDriver && enabled) {
        tmcDriver->semin(1);
        tmcDriver->semax(0);
        tmcDriver->seup(1);
        tmcDriver->sedn(0);
    }
}

bool TMC2209Driver::isCoolStepEnabled() const {
    return coolStepEnabled;
}

void TMC2209Driver::setStallThreshold(int8_t threshold) {
    if (tmcDriver) {
        tmcDriver->SGTHRS(threshold);
    }
}

int8_t TMC2209Driver::getStallThreshold() const {
    if (tmcDriver) {
        return tmcDriver->SGTHRS();
    }
    return 0;
}

bool TMC2209Driver::isStallDetected() const {
    if (tmcDriver) {
        return tmcDriver->diag();
    }
    return false;
}

// Diagnostics
uint32_t TMC2209Driver::getDriverStatus() const {
    if (tmcDriver) {
        return tmcDriver->DRV_STATUS();
    }
    return 0;
}

float TMC2209Driver::getSupplyVoltage() const {
    // TMC2209 doesn't directly measure supply voltage
    // This would need external ADC measurement
    return 12.0f; // Default assumption
}

float TMC2209Driver::getDriverTemperature() const {
    if (tmcDriver) {
        if (tmcDriver->ot()) {
            return 150.0f; // Over temperature shutdown
        } else if (tmcDriver->otpw()) {
            return 120.0f; // Over temperature warning
        }
    }
    return 25.0f; // Normal operating temperature
}

// Status string for debugging
String TMC2209Driver::getTMCStatusString() const {
    if (!tmcDriver) return "TMC2209 not initialized";

    String status = "TMC2209 Status:\n";
    status += "  Microsteps: " + String(microsteps) + "\n";
    status += "  Current: " + String(currentMA) + " mA\n";
    status += "  Mode: " + String(stealthChopEnabled ? "StealthChop" : "SpreadCycle") + "\n";

    // Temperature status
    if (tmcDriver->ot()) {
        status += "  TEMP: SHUTDOWN (>150°C)\n";
    } else if (tmcDriver->otpw()) {
        status += "  TEMP: WARNING (>120°C)\n";
    } else {
        status += "  TEMP: Normal\n";
    }

    // Error flags
    if (tmcDriver->s2ga() || tmcDriver->s2gb()) {
        status += "  ERROR: Short to ground!\n";
    }
    if (tmcDriver->ola() || tmcDriver->olb()) {
        status += "  ERROR: Open load!\n";
    }
    if (tmcDriver->diag()) {
        status += "  WARNING: Stall detected!\n";
    }

    status += "  Load: " + String((tmcDriver->cs_actual() * 100) / 31) + "%\n";
    status += "  StallGuard: " + String(tmcDriver->SG_RESULT());

    return status;
}

bool TMC2209Driver::checkTMCCommunication() const {
    if (!tmcDriver) return false;

    // Try to read a register to verify communication
    tmcDriver->toff(5);
    return (tmcDriver->toff() == 5);
}

// Helper methods for EEPROM
void TMC2209Driver::saveConfigToEEPROM() {
    EEPROM.write(EEPROM_TMC_CONFIG_FLAG, 0xFF);
    EEPROM.put(EEPROM_TMC_MICROSTEPS, microsteps);
    EEPROM.put(EEPROM_TMC_CURRENT, currentMA);
    EEPROM.write(EEPROM_TMC_STEALTHCHOP, stealthChopEnabled ? 1 : 0);
    EEPROM.commit();
}

void TMC2209Driver::loadConfigFromEEPROM() {
    if (EEPROM.read(EEPROM_TMC_CONFIG_FLAG) == 0xFF) {
        uint16_t savedMicrosteps, savedCurrent;
        EEPROM.get(EEPROM_TMC_MICROSTEPS, savedMicrosteps);
        EEPROM.get(EEPROM_TMC_CURRENT, savedCurrent);
        stealthChopEnabled = (EEPROM.read(EEPROM_TMC_STEALTHCHOP) == 1);

        // Validate loaded values
        if (savedMicrosteps == 1 || savedMicrosteps == 2 || savedMicrosteps == 4 ||
            savedMicrosteps == 8 || savedMicrosteps == 16 || savedMicrosteps == 32 ||
            savedMicrosteps == 64 || savedMicrosteps == 128 || savedMicrosteps == 256) {
            microsteps = savedMicrosteps;
        }

        if (savedCurrent >= MIN_MOTOR_CURRENT && savedCurrent <= MAX_MOTOR_CURRENT) {
            currentMA = savedCurrent;
        }

        Serial.println("TMC2209 configuration loaded from EEPROM");
    }
}

#endif // MOTOR_DRIVER_TMC2209