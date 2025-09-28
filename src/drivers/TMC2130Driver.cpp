#include "TMC2130Driver.h"
#include "../config.h"
#include <Arduino.h>
#include <EEPROM.h>

#ifdef MOTOR_DRIVER_TMC2130

// Constructor
TMC2130Driver::TMC2130Driver(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin, uint8_t csPin)
    : stepPin(stepPin), dirPin(dirPin), enablePin(enablePin), csPin(csPin)
    , tmcDriver(nullptr), stepper(nullptr)
    , motorEnabled(false), directionReversed(false)
    , currentPosition(0), targetPosition(0), isMoving(false)
    , microsteps(DEFAULT_MICROSTEPS), currentMA(DEFAULT_MOTOR_CURRENT)
    , stealthChopEnabled(USE_STEALTHCHOP), stallGuardEnabled(USE_STALLGUARD)
    , stallGuardThreshold(STALLGUARD_THRESHOLD)
    , speed(MOTOR_SPEED), maxSpeed(MAX_MOTOR_SPEED), acceleration(MOTOR_ACCELERATION)
{
}

// Initialize the TMC2130 driver
void TMC2130Driver::init() {
    // Configure pins
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    pinMode(csPin, OUTPUT);
    digitalWrite(enablePin, HIGH); // Disable motor initially (active low)
    digitalWrite(csPin, HIGH);     // Deselect chip

    // Initialize SPI
    SPI.begin();

    // Create TMC2130 driver instance
    tmcDriver = new TMC2130Stepper(csPin, TMC_R_SENSE);

    // Initialize TMC2130
    tmcDriver->begin();

    // Test communication
    tmcDriver->toff(5); // Enable driver
    uint8_t test = tmcDriver->toff();
    if (test != 5) {
        Serial.println("ERROR: TMC2130 communication failed!");
        Serial.println("Check SPI connections and power supply.");
        return;
    }

    // Load configuration from EEPROM if available
    loadConfigFromEEPROM();

    // Apply configuration
    tmcDriver->rms_current(currentMA);
    tmcDriver->microsteps(microsteps);

    // Configure StealthChop or SpreadCycle
    if (stealthChopEnabled) {
        tmcDriver->en_pwm_mode(true);      // Enable StealthChop
        tmcDriver->pwm_autoscale(true);
        tmcDriver->pwm_freq(2);            // PWM frequency 2/1024 fclk
        tmcDriver->pwm_grad(5);            // PWM gradient
        tmcDriver->pwm_ampl(255);          // PWM amplitude
        tmcDriver->TPWMTHRS(STEALTHCHOP_THRESHOLD);
    } else {
        tmcDriver->en_pwm_mode(false);     // Use SpreadCycle
    }

    // Set hold current multiplier
    tmcDriver->hold_multiplier(MOTOR_HOLD_MULTIPLIER);

    // Configure StallGuard if enabled
    if (stallGuardEnabled) {
        tmcDriver->sgt(stallGuardThreshold);
        tmcDriver->sfilt(1);  // Enable StallGuard filter
    }

    // Configure coolstep for automatic current adaptation
    tmcDriver->semin(5);     // Minimum current for smart current control
    tmcDriver->semax(2);     // Maximum current for smart current control
    tmcDriver->sedn(0b01);   // Current down step

    // Initialize AccelStepper for movement control
    stepper = new AccelStepper(AccelStepper::DRIVER, stepPin, dirPin);
    stepper->setMaxSpeed(maxSpeed * microsteps);
    stepper->setAcceleration(acceleration * microsteps);
    stepper->setSpeed(speed * microsteps);

    Serial.println("TMC2130 initialized successfully");
    Serial.print("  Current: "); Serial.print(currentMA); Serial.println(" mA");
    Serial.print("  Microsteps: "); Serial.println(microsteps);
    Serial.print("  Mode: "); Serial.println(stealthChopEnabled ? "StealthChop" : "SpreadCycle");
    Serial.print("  StallGuard: "); Serial.println(stallGuardEnabled ? "Enabled" : "Disabled");
}

// Movement methods
void TMC2130Driver::move(long steps) {
    if (!tmcDriver || !stepper) return;

    long actualSteps = steps * microsteps;
    stepper->move(actualSteps);
    targetPosition = currentPosition + steps;
    isMoving = true;
    enableMotor();
}

void TMC2130Driver::moveTo(long position) {
    if (!tmcDriver || !stepper) return;

    long actualPosition = position * microsteps;
    stepper->moveTo(actualPosition);
    targetPosition = position;
    isMoving = true;
    enableMotor();
}

void TMC2130Driver::setCurrentPosition(long position) {
    if (!stepper) return;

    currentPosition = position;
    targetPosition = position;
    stepper->setCurrentPosition(position * microsteps);
    isMoving = false;
}

long TMC2130Driver::getCurrentPosition() const {
    if (!stepper) return 0;
    return stepper->currentPosition() / microsteps;
}

long TMC2130Driver::getTargetPosition() const {
    return targetPosition;
}

bool TMC2130Driver::run() {
    if (!stepper || !motorEnabled) return false;

    bool stillRunning = stepper->run();

    if (stillRunning) {
        // Update current position
        currentPosition = stepper->currentPosition() / microsteps;

        // Check for stall detection
        if (stallGuardEnabled && tmcDriver) {
            uint32_t sgResult = tmcDriver->sg_result();
            if (sgResult == 0) {  // Stall detected when sg_result reaches 0
                Serial.println("WARNING: Stall detected!");
                emergencyStop();
                return false;
            }
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

void TMC2130Driver::runToPosition() {
    if (!stepper) return;

    enableMotor();
    while (stepper->distanceToGo() != 0) {
        stepper->run();

        // Check for stall
        if (stallGuardEnabled && tmcDriver) {
            uint32_t sgResult = tmcDriver->sg_result();
            if (sgResult == 0) {
                Serial.println("ERROR: Stall detected during movement!");
                emergencyStop();
                break;
            }
        }
    }
    currentPosition = targetPosition;
    isMoving = false;
}

bool TMC2130Driver::isRunning() const {
    if (!stepper) return false;
    return stepper->isRunning();
}

void TMC2130Driver::stop() {
    if (!stepper) return;
    stepper->stop();
    isMoving = false;
}

void TMC2130Driver::emergencyStop() {
    if (stepper) {
        stepper->stop();
        stepper->setCurrentPosition(stepper->currentPosition());
    }
    isMoving = false;
    disableMotor();
}

// Speed and acceleration methods
void TMC2130Driver::setSpeed(float speed) {
    this->speed = speed;
    if (stepper) {
        stepper->setSpeed(speed * microsteps);
    }
}

void TMC2130Driver::setMaxSpeed(float maxSpeed) {
    this->maxSpeed = maxSpeed;
    if (stepper) {
        stepper->setMaxSpeed(maxSpeed * microsteps);
    }
}

void TMC2130Driver::setAcceleration(float acceleration) {
    this->acceleration = acceleration;
    if (stepper) {
        stepper->setAcceleration(acceleration * microsteps);
    }
}

float TMC2130Driver::getSpeed() const {
    return speed;
}

float TMC2130Driver::getMaxSpeed() const {
    return maxSpeed;
}

float TMC2130Driver::getAcceleration() const {
    return acceleration;
}

// Motor enable/disable
void TMC2130Driver::enableMotor() {
    motorEnabled = true;
    digitalWrite(enablePin, LOW); // Active low
}

void TMC2130Driver::disableMotor() {
    motorEnabled = false;
    digitalWrite(enablePin, HIGH); // Disable
}

bool TMC2130Driver::isMotorEnabled() const {
    return motorEnabled;
}

// Direction control
void TMC2130Driver::setDirectionReversed(bool reversed) {
    directionReversed = reversed;
    if (stepper) {
        stepper->setPinsInverted(reversed, false, false);
    }
}

bool TMC2130Driver::isDirectionReversed() const {
    return directionReversed;
}

// TMC2130-specific configuration
void TMC2130Driver::setMicrosteps(uint16_t microsteps) {
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

uint16_t TMC2130Driver::getMicrosteps() const {
    return microsteps;
}

void TMC2130Driver::setCurrent(uint16_t currentMA) {
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

uint16_t TMC2130Driver::getCurrent() const {
    return currentMA;
}

void TMC2130Driver::setStealthChopEnabled(bool enabled) {
    stealthChopEnabled = enabled;

    if (tmcDriver) {
        if (enabled) {
            tmcDriver->en_pwm_mode(true);
            tmcDriver->pwm_autoscale(true);
            tmcDriver->pwm_freq(2);
            tmcDriver->pwm_grad(5);
            tmcDriver->pwm_ampl(255);
            tmcDriver->TPWMTHRS(STEALTHCHOP_THRESHOLD);
        } else {
            tmcDriver->en_pwm_mode(false);
        }
    }

    saveConfigToEEPROM();
    Serial.print("StealthChop: "); Serial.println(enabled ? "Enabled" : "Disabled");
}

bool TMC2130Driver::isStealthChopEnabled() const {
    return stealthChopEnabled;
}

// TMC2130 unique features
void TMC2130Driver::setStallGuardEnabled(bool enabled) {
    stallGuardEnabled = enabled;

    if (tmcDriver && enabled) {
        tmcDriver->sgt(stallGuardThreshold);
        tmcDriver->sfilt(1);
    }

    saveConfigToEEPROM();
    Serial.print("StallGuard: "); Serial.println(enabled ? "Enabled" : "Disabled");
}

bool TMC2130Driver::isStallGuardEnabled() const {
    return stallGuardEnabled;
}

void TMC2130Driver::setStallGuardThreshold(int8_t threshold) {
    stallGuardThreshold = threshold;
    if (tmcDriver) {
        tmcDriver->sgt(threshold);
    }
    saveConfigToEEPROM();
    Serial.print("StallGuard threshold set to: "); Serial.println(threshold);
}

int8_t TMC2130Driver::getStallGuardThreshold() const {
    return stallGuardThreshold;
}

bool TMC2130Driver::isStallDetected() const {
    if (tmcDriver && stallGuardEnabled) {
        return (tmcDriver->sg_result() == 0);
    }
    return false;
}

// Diagnostics
uint32_t TMC2130Driver::getDriverStatus() const {
    if (tmcDriver) {
        return tmcDriver->DRV_STATUS();
    }
    return 0;
}

float TMC2130Driver::getSupplyVoltage() const {
    // TMC2130 doesn't directly measure supply voltage
    return 12.0f; // Default assumption
}

float TMC2130Driver::getDriverTemperature() const {
    if (tmcDriver) {
        if (tmcDriver->otpw()) {
            return 120.0f; // Over temperature warning
        }
    }
    return 25.0f; // Normal operating temperature
}

uint32_t TMC2130Driver::getLoadValue() const {
    if (tmcDriver) {
        return tmcDriver->sg_result();
    }
    return 0;
}

// Status string for debugging
String TMC2130Driver::getTMCStatusString() const {
    if (!tmcDriver) return "TMC2130 not initialized";

    String status = "TMC2130 Status:\n";
    status += "  Microsteps: " + String(microsteps) + "\n";
    status += "  Current: " + String(currentMA) + " mA\n";
    status += "  Mode: " + String(stealthChopEnabled ? "StealthChop" : "SpreadCycle") + "\n";
    status += "  StallGuard: " + String(stallGuardEnabled ? "Enabled" : "Disabled") + "\n";

    // Temperature status
    if (tmcDriver->otpw()) {
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

    // StallGuard value (higher = less load, 0 = stall)
    status += "  StallGuard Value: " + String(tmcDriver->sg_result()) + "/1023\n";

    // Load percentage (inverse of StallGuard)
    float loadPercent = 100.0f - (tmcDriver->sg_result() * 100.0f / 1023.0f);
    status += "  Load: " + String(loadPercent, 1) + "%";

    return status;
}

bool TMC2130Driver::checkTMCCommunication() const {
    if (!tmcDriver) return false;

    // Try to read a register to verify communication
    tmcDriver->toff(5);
    return (tmcDriver->toff() == 5);
}

// Sensorless homing using StallGuard
bool TMC2130Driver::performSensorlessHoming(bool direction, uint32_t timeoutMs) {
    if (!tmcDriver || !stepper) return false;

    Serial.println("Starting sensorless homing...");

    // Save current settings
    bool savedStallGuard = stallGuardEnabled;
    int8_t savedThreshold = stallGuardThreshold;

    // Enable StallGuard for homing
    setStallGuardEnabled(true);
    setStallGuardThreshold(8);  // Sensitive threshold for homing

    // Set direction and enable motor
    stepper->setSpeed(direction ? 200 : -200);  // Slow speed for homing
    enableMotor();

    unsigned long startTime = millis();
    bool stalled = false;

    // Move until stall or timeout
    while ((millis() - startTime) < timeoutMs) {
        stepper->runSpeed();

        if (tmcDriver->sg_result() == 0) {
            stalled = true;
            Serial.println("Stall detected - home position found!");
            break;
        }

        delay(1);
    }

    // Stop motor
    stop();

    if (stalled) {
        // Set this as home position
        setCurrentPosition(0);
        Serial.println("Homing successful!");
    } else {
        Serial.println("Homing timeout!");
    }

    // Restore settings
    setStallGuardEnabled(savedStallGuard);
    setStallGuardThreshold(savedThreshold);

    return stalled;
}

// Helper methods for EEPROM
void TMC2130Driver::saveConfigToEEPROM() {
    EEPROM.write(EEPROM_TMC_CONFIG_FLAG, 0xFF);
    EEPROM.put(EEPROM_TMC_MICROSTEPS, microsteps);
    EEPROM.put(EEPROM_TMC_CURRENT, currentMA);
    EEPROM.write(EEPROM_TMC_STEALTHCHOP, stealthChopEnabled ? 1 : 0);
    // Save TMC2130-specific settings at different addresses
    EEPROM.write(EEPROM_TMC_CONFIG_FLAG + 1, stallGuardEnabled ? 1 : 0);
    EEPROM.write(EEPROM_TMC_CONFIG_FLAG + 2, stallGuardThreshold);
    EEPROM.commit();
}

void TMC2130Driver::loadConfigFromEEPROM() {
    if (EEPROM.read(EEPROM_TMC_CONFIG_FLAG) == 0xFF) {
        uint16_t savedMicrosteps, savedCurrent;
        EEPROM.get(EEPROM_TMC_MICROSTEPS, savedMicrosteps);
        EEPROM.get(EEPROM_TMC_CURRENT, savedCurrent);
        stealthChopEnabled = (EEPROM.read(EEPROM_TMC_STEALTHCHOP) == 1);

        // Load TMC2130-specific settings
        stallGuardEnabled = (EEPROM.read(EEPROM_TMC_CONFIG_FLAG + 1) == 1);
        stallGuardThreshold = EEPROM.read(EEPROM_TMC_CONFIG_FLAG + 2);

        // Validate loaded values
        if (savedMicrosteps == 1 || savedMicrosteps == 2 || savedMicrosteps == 4 ||
            savedMicrosteps == 8 || savedMicrosteps == 16 || savedMicrosteps == 32 ||
            savedMicrosteps == 64 || savedMicrosteps == 128 || savedMicrosteps == 256) {
            microsteps = savedMicrosteps;
        }

        if (savedCurrent >= MIN_MOTOR_CURRENT && savedCurrent <= MAX_MOTOR_CURRENT) {
            currentMA = savedCurrent;
        }

        if (stallGuardThreshold < -64 || stallGuardThreshold > 63) {
            stallGuardThreshold = STALLGUARD_THRESHOLD;
        }

        Serial.println("TMC2130 configuration loaded from EEPROM");
    }
}

#endif // MOTOR_DRIVER_TMC2130