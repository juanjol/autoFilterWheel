#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <EEPROM.h>
#include "config.h"

// ============================================
// GLOBAL OBJECTS
// ============================================

// OLED Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Stepper motor with ULN2003 driver (28BYJ-48)
AccelStepper stepper(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

// ============================================
// GLOBAL VARIABLES
// ============================================

// Dynamic filter count and position tracking
uint8_t numFilters = NUM_FILTERS;    // Dynamic filter count (loaded from EEPROM)
uint8_t currentPosition = 1;         // Current filter position (1 to numFilters)
uint8_t targetPosition = 1;

// AS5600 encoder variables
float currentAngle = 0.0;
float angleOffset = 0.0;
bool encoderAvailable = false;

// System status
bool isMoving = false;
bool isCalibrated = false;
uint8_t errorCode = ERROR_NONE;
bool motorEnabled = true;

// Motor power management
unsigned long motorDisableTime = 0;
bool motorDisablePending = false;

// Timing variables
unsigned long lastDisplayUpdate = 0;
unsigned long movementStartTime = 0;

// Serial command buffer
String commandBuffer = "";

// Revolution calibration variables
bool revCalibrationMode = false;
uint16_t revCalibrationSteps = 0;
uint16_t calibratedStepsPerRevolution = STEPS_PER_REVOLUTION;
long revCalibrationStartPosition = 0;

// Backlash calibration variables
bool backlashCalibrationMode = false;
uint8_t calibratedBacklashSteps = BACKLASH_COMPENSATION;
uint8_t backlashTestSteps = 0;
uint8_t forwardBacklash = 0;
long backlashTestStartPosition = 0;
bool backlashDirectionForward = true;

// Motor direction tracking for backlash compensation
int lastMotorDirection = 0; // 1 = forward, -1 = backward, 0 = unknown

// Filter position angles (calculated dynamically)
float filterAngles[MAX_FILTER_COUNT];

// Filter names array - dynamic storage
char filterNames[MAX_FILTER_COUNT][MAX_FILTER_NAME_LENGTH + 1];

// ============================================
// FUNCTION DECLARATIONS
// ============================================
int getStepsPerFilter();
int applyBacklashCompensation(int steps);

// Default filter names (fallback)
const char* defaultFilterNames[MAX_FILTER_COUNT] = {
    FILTER_NAME_1,
    FILTER_NAME_2,
    FILTER_NAME_3,
    FILTER_NAME_4,
    FILTER_NAME_5,
    "Filter 6",  // Additional defaults for expanded filter counts
    "Filter 7",
    "Filter 8"
};

// Get filter name for a position
const char* getFilterName(uint8_t position) {
    if (position >= 1 && position <= numFilters) {
        return filterNames[position - 1];
    }
    return "?";
}

// ============================================
// AS5600 FUNCTIONS
// ============================================

void initAS5600() {
    Wire.beginTransmission(AS5600_ADDRESS);
    if (Wire.endTransmission() == 0) {
        encoderAvailable = true;
        if (DEBUG_MODE) {
            Serial.println("AS5600 detected");
        }
    } else {
        encoderAvailable = false;
        if (DEBUG_MODE) {
            Serial.println("AS5600 not found - running in stepper-only mode");
        }
    }
}

float readAS5600Angle() {
    if (!encoderAvailable) return 0.0;

    Wire.beginTransmission(AS5600_ADDRESS);
    Wire.write(AS5600_RAW_ANGLE_REGISTER);
    Wire.endTransmission(false);

    Wire.requestFrom(AS5600_ADDRESS, 2);
    if (Wire.available() >= 2) {
        uint16_t rawAngle = Wire.read() << 8;
        rawAngle |= Wire.read();

        // Convert to degrees (0-360)
        float angle = (rawAngle * 360.0) / 4096.0;

        // Apply offset
        angle = fmod(angle + angleOffset, 360.0);
        if (angle < 0) angle += 360.0;

        return angle;
    }
    return 0.0;
}

uint8_t angleToPosition(float angle) {
    // Find closest filter position based on angle
    uint8_t closestPosition = 1;
    float minDifference = 360.0;

    for (uint8_t i = 0; i < numFilters; i++) {
        float diff = abs(angle - filterAngles[i]);
        // Handle wrap-around at 360 degrees
        if (diff > 180.0) diff = 360.0 - diff;

        if (diff < minDifference) {
            minDifference = diff;
            closestPosition = i + 1;
        }
    }

    // Check if within tolerance
    if (minDifference <= ANGLE_TOLERANCE) {
        return closestPosition;
    }

    return 0; // Position uncertain
}

// ============================================
// EEPROM FUNCTIONS
// ============================================

void loadCalibration() {
    EEPROM.begin(EEPROM_SIZE);

    uint8_t calibFlag = EEPROM.read(EEPROM_CALIB_FLAG);
    if (calibFlag == 0xAA) {
        // Load calibration data
        EEPROM.get(EEPROM_OFFSET_ADDR, angleOffset);
        isCalibrated = true;
        if (DEBUG_MODE) {
            Serial.println("Calibration loaded from EEPROM");
        }
    } else {
        if (DEBUG_MODE) {
            Serial.println("No calibration found");
        }
    }
}

void saveCalibration() {
    EEPROM.write(EEPROM_CALIB_FLAG, 0xAA);
    EEPROM.put(EEPROM_OFFSET_ADDR, angleOffset);
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.println("Calibration saved to EEPROM");
    }
}

void saveCurrentPosition() {
    EEPROM.write(EEPROM_CURRENT_POS, currentPosition);
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.print("Current position saved: ");
        Serial.println(currentPosition);
    }
}

void loadCurrentPosition() {
    uint8_t savedPosition = EEPROM.read(EEPROM_CURRENT_POS);
    if (savedPosition >= 1 && savedPosition <= numFilters) {
        currentPosition = savedPosition;
        stepper.setCurrentPosition((currentPosition - 1) * getStepsPerFilter());
        if (DEBUG_MODE) {
            Serial.print("Restored position from EEPROM: ");
            Serial.println(currentPosition);
        }
    } else {
        // Invalid saved position, default to position 1
        currentPosition = 1;
        stepper.setCurrentPosition(0);
        if (DEBUG_MODE) {
            Serial.println("No valid saved position, defaulting to position 1");
        }
    }
}

void saveFilterNames() {
    EEPROM.write(EEPROM_FILTER_NAMES_FLAG, 0xBB); // Magic byte for filter names
    for (int i = 0; i < numFilters; i++) {
        int addr = EEPROM_FILTER_NAMES_ADDR + (i * 16);
        for (int j = 0; j < 16; j++) {
            if (j < strlen(filterNames[i])) {
                EEPROM.write(addr + j, filterNames[i][j]);
            } else {
                EEPROM.write(addr + j, 0); // Null terminator and padding
            }
        }
    }
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.println("Filter names saved to EEPROM");
    }
}

void loadFilterNames() {
    uint8_t namesFlag = EEPROM.read(EEPROM_FILTER_NAMES_FLAG);
    if (namesFlag == 0xBB) {
        // Load custom filter names from EEPROM
        for (int i = 0; i < numFilters; i++) {
            int addr = EEPROM_FILTER_NAMES_ADDR + (i * 16);
            for (int j = 0; j < MAX_FILTER_NAME_LENGTH; j++) {
                filterNames[i][j] = EEPROM.read(addr + j);
                if (filterNames[i][j] == 0) break; // Stop at null terminator
            }
            filterNames[i][MAX_FILTER_NAME_LENGTH] = 0; // Ensure null termination
        }
        if (DEBUG_MODE) {
            Serial.println("Custom filter names loaded from EEPROM");
        }
    } else {
        // Use default filter names
        for (int i = 0; i < numFilters; i++) {
            strncpy(filterNames[i], defaultFilterNames[i], MAX_FILTER_NAME_LENGTH);
            filterNames[i][MAX_FILTER_NAME_LENGTH] = 0; // Ensure null termination
        }
        if (DEBUG_MODE) {
            Serial.println("Using default filter names");
        }
    }
}

void saveFilterCount() {
    EEPROM.write(EEPROM_FILTER_COUNT, numFilters);
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.print("Filter count saved: ");
        Serial.println(numFilters);
    }
}

void loadFilterCount() {
    uint8_t savedCount = EEPROM.read(EEPROM_FILTER_COUNT);
    if (savedCount >= MIN_FILTER_COUNT && savedCount <= MAX_FILTER_COUNT) {
        numFilters = savedCount;
        if (DEBUG_MODE) {
            Serial.print("Loaded filter count from EEPROM: ");
            Serial.println(numFilters);
        }
    } else {
        // Use default count
        numFilters = NUM_FILTERS;
        if (DEBUG_MODE) {
            Serial.print("Using default filter count: ");
            Serial.println(numFilters);
        }
    }

}

// Save revolution calibration to EEPROM
void saveRevolutionCalibration() {
    EEPROM.write(EEPROM_REV_CAL_FLAG, 0xBB); // Magic byte for rev calibration
    EEPROM.put(EEPROM_STEPS_PER_REV, calibratedStepsPerRevolution);
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.print("Revolution calibration saved: ");
        Serial.println(calibratedStepsPerRevolution);
    }
}

// Load revolution calibration from EEPROM
void loadRevolutionCalibration() {
    if (EEPROM.read(EEPROM_REV_CAL_FLAG) == 0xBB) {
        EEPROM.get(EEPROM_STEPS_PER_REV, calibratedStepsPerRevolution);
        if (DEBUG_MODE) {
            Serial.print("Loaded revolution calibration: ");
            Serial.println(calibratedStepsPerRevolution);
        }
    } else {
        calibratedStepsPerRevolution = STEPS_PER_REVOLUTION; // Use default
        if (DEBUG_MODE) {
            Serial.print("Using default steps per revolution: ");
            Serial.println(calibratedStepsPerRevolution);
        }
    }
}

// Save backlash calibration to EEPROM
void saveBacklashCalibration() {
    EEPROM.write(EEPROM_BACKLASH_FLAG, 0xCC); // Magic byte for backlash calibration
    EEPROM.write(EEPROM_BACKLASH_STEPS, calibratedBacklashSteps);
    EEPROM.commit();
    if (DEBUG_MODE) {
        Serial.print("Backlash calibration saved: ");
        Serial.println(calibratedBacklashSteps);
    }
}

// Load backlash calibration from EEPROM
void loadBacklashCalibration() {
    if (EEPROM.read(EEPROM_BACKLASH_FLAG) == 0xCC) {
        calibratedBacklashSteps = EEPROM.read(EEPROM_BACKLASH_STEPS);
        if (DEBUG_MODE) {
            Serial.print("Loaded backlash calibration: ");
            Serial.println(calibratedBacklashSteps);
        }
    } else {
        calibratedBacklashSteps = BACKLASH_COMPENSATION; // Use default
        if (DEBUG_MODE) {
            Serial.print("Using default backlash compensation: ");
            Serial.println(calibratedBacklashSteps);
        }
    }
}

// Calculate steps per filter dynamically
int getStepsPerFilter() {
    return calibratedStepsPerRevolution / numFilters;
}

// Apply backlash compensation when changing direction
int applyBacklashCompensation(int steps) {
    if (steps == 0) return 0;

    int currentDirection = (steps > 0) ? 1 : -1;
    int compensatedSteps = steps;

    // Apply backlash compensation if changing direction from forward to backward
    if (lastMotorDirection == 1 && currentDirection == -1) {
        compensatedSteps -= calibratedBacklashSteps;
        if (DEBUG_MODE) {
            Serial.print("Backlash compensation applied: -");
            Serial.println(calibratedBacklashSteps);
        }
    }

    // Update direction tracking
    lastMotorDirection = currentDirection;

    return compensatedSteps;
}

// ============================================
// MOTOR POWER MANAGEMENT FUNCTIONS
// ============================================

void enableMotor() {
    if (!motorEnabled) {
        motorEnabled = true;  // Set flag FIRST before enabling outputs
        motorDisablePending = false;

        if (MOTOR_ENABLE_PIN >= 0) {
            digitalWrite(MOTOR_ENABLE_PIN, HIGH);
        }

        // Re-energize motor coils to current position
        stepper.enableOutputs();

        if (DEBUG_MODE) {
            Serial.println("Motor enabled");
        }
    }
}

void forceMotorOff() {
    // Directly control pins without using stepper library
    digitalWrite(MOTOR_PIN1, LOW);
    delayMicroseconds(10);
    digitalWrite(MOTOR_PIN2, LOW);
    delayMicroseconds(10);
    digitalWrite(MOTOR_PIN3, LOW);
    delayMicroseconds(10);
    digitalWrite(MOTOR_PIN4, LOW);
    delayMicroseconds(10);
}

void disableMotor() {
    if (motorEnabled) {
        motorEnabled = false;  // Set flag FIRST to prevent stepper.run()
        motorDisablePending = false;

        // Stop any ongoing movement
        stepper.stop();

        // Reset to current position
        long currentPos = stepper.currentPosition();
        stepper.setCurrentPosition(currentPos);
        stepper.moveTo(currentPos);  // Ensure target = current

        // Disable motor outputs
        stepper.disableOutputs();

        // Multiple attempts to ensure all pins are LOW
        for (int i = 0; i < 5; i++) {
            forceMotorOff();
            delay(5);
        }

        if (MOTOR_ENABLE_PIN >= 0) {
            digitalWrite(MOTOR_ENABLE_PIN, LOW);
        }

        if (DEBUG_MODE) {
            Serial.println("Motor disabled - power saving mode");
            Serial.print("Pin states: ");
            Serial.print(digitalRead(MOTOR_PIN1));
            Serial.print(",");
            Serial.print(digitalRead(MOTOR_PIN2));
            Serial.print(",");
            Serial.print(digitalRead(MOTOR_PIN3));
            Serial.print(",");
            Serial.println(digitalRead(MOTOR_PIN4));
        }
    }
}

void scheduleMotorDisable() {
    if (AUTO_DISABLE_MOTOR && motorEnabled) {
        motorDisableTime = millis() + MOTOR_DISABLE_DELAY;
        motorDisablePending = true;

        if (DEBUG_MODE) {
            Serial.print("Motor disable scheduled in ");
            Serial.print(MOTOR_DISABLE_DELAY);
            Serial.println("ms");
        }
    }
}

void handleMotorPowerManagement() {
    // Check if motor should be disabled
    if (motorDisablePending && millis() >= motorDisableTime && !isMoving) {
        disableMotor();
    }
}

// ============================================
// DISPLAY FUNCTIONS
// ============================================

void updateDisplay() {
    if (millis() - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) return;
    lastDisplayUpdate = millis();

    // Display optimized for 0.42" OLED (72x40 visible area)
    display.clearDisplay();

    // Line 1 - Status (small font)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(OLED_X_OFFSET, 24);

    if (isMoving) {
        display.print("MOVING");
        if (targetPosition > 0) {
            display.print(" -> ");
            display.print(targetPosition);
        }
    } else if (!isCalibrated) {
        display.print("NOT CALIBRATED");
    } else if (!motorEnabled) {
        display.print("MOTOR OFF");
    } else if (errorCode != ERROR_NONE) {
        display.print("ERROR: ");
        display.print(errorCode);
    } else {
        display.print("READY");
    }

    // Line 2 - Position (bigger font)
    display.setTextSize(2);
    display.setCursor(OLED_X_OFFSET + 10, 36);
    display.print("POS ");
    display.print(currentPosition);

    // Line 3 - Filter name (small font)
    display.setTextSize(1);
    display.setCursor(OLED_X_OFFSET, 52);

    const char* filterName = getFilterName(currentPosition);
    int maxLen = 10;  // Maximum characters to show
    for (int i = 0; i < maxLen && filterName[i] != '\0'; i++) {
        display.print(filterName[i]);
    }

    display.display();
}

// ============================================
// MOTOR CONTROL FUNCTIONS
// ============================================

void moveSteps(int steps) {
    if (steps == 0) return;

    // Validate step count
    if (abs(steps) > MAX_MANUAL_STEPS) {
        errorCode = ERROR_INVALID_POSITION;
        Serial.print("ERROR:MAX_STEPS_EXCEEDED (max: ");
        Serial.print(MAX_MANUAL_STEPS);
        Serial.println(")");
        return;
    }

    // Enable motor before movement
    enableMotor();

    isMoving = true;
    movementStartTime = millis();
    errorCode = ERROR_NONE;

    // Apply backlash compensation
    int compensatedSteps = applyBacklashCompensation(steps);
    // Move motor the specified steps
    stepper.move(compensatedSteps);

    if (DEBUG_MODE) {
        Serial.print("Manual stepping: ");
        Serial.print(steps);
        if (compensatedSteps != steps) {
            Serial.print(" steps (compensated: ");
            Serial.print(compensatedSteps);
            Serial.print(")");
        } else {
            Serial.print(" steps");
        }
        Serial.println();
    }
}

void moveToStep(long absoluteStep) {
    // Calculate relative movement
    long currentStep = stepper.currentPosition();
    long stepsToMove = absoluteStep - currentStep;

    // Validate step count
    if (abs(stepsToMove) > MAX_MANUAL_STEPS) {
        errorCode = ERROR_INVALID_POSITION;
        Serial.print("ERROR:MOVEMENT_TOO_LARGE (would move ");
        Serial.print(abs(stepsToMove));
        Serial.println(" steps)");
        return;
    }

    // Enable motor before movement
    enableMotor();

    // Move to absolute step position
    isMoving = true;
    movementStartTime = millis();
    errorCode = ERROR_NONE;

    stepper.moveTo(absoluteStep);

    if (DEBUG_MODE) {
        Serial.print("Moving to absolute step: ");
        Serial.print(absoluteStep);
        Serial.print(" (current: ");
        Serial.print(currentStep);
        Serial.print(", delta: ");
        Serial.print(stepsToMove);
        Serial.println(")");
    }
}

long getCurrentStepPosition() {
    return stepper.currentPosition();
}

void moveToPosition(uint8_t position) {
    if (position < 1 || position > numFilters) {
        errorCode = ERROR_INVALID_POSITION;
        return;
    }

    // Enable motor before movement
    enableMotor();

    targetPosition = position;
    isMoving = true;
    movementStartTime = millis();
    errorCode = ERROR_NONE;

    // Calculate steps needed
    int currentSteps = (currentPosition - 1) * getStepsPerFilter();
    int targetSteps = (position - 1) * getStepsPerFilter();
    int stepsToMove = targetSteps - currentSteps;

    if (MOTOR_DIRECTION_MODE == 0) {
        // UNIDIRECTIONAL MODE - Always move in one direction
        if (targetPosition < currentPosition) {
            // Need to go forward through zero
            // For example: from position 4 to position 2 = forward through 5, 1, to 2
            // That's 2 steps forward (4->5->1) + 1 more step (1->2) = 3 total filter positions
            int positionsToMove = (numFilters - currentPosition) + targetPosition;
            stepsToMove = positionsToMove * getStepsPerFilter();
        } else if (targetPosition == currentPosition) {
            // Already at target
            isMoving = false;
            return;
        } else {
            // Normal forward movement
            int positionsToMove = targetPosition - currentPosition;
            stepsToMove = positionsToMove * getStepsPerFilter();
        }

        // Apply direction reversal if configured
        if (MOTOR_REVERSE_DIRECTION) {
            stepsToMove = -stepsToMove;
        }

    } else {
        // BIDIRECTIONAL MODE - Take shortest path
        // Handle shortest path (considering circular movement)
        if (abs(stepsToMove) > STEPS_PER_REVOLUTION / 2) {
            if (stepsToMove > 0) {
                stepsToMove -= STEPS_PER_REVOLUTION;
            } else {
                stepsToMove += STEPS_PER_REVOLUTION;
            }
        }

    }

    // Apply backlash compensation
    int compensatedSteps = applyBacklashCompensation(stepsToMove);
    // Move motor
    stepper.move(compensatedSteps);

    if (DEBUG_MODE) {
        Serial.print("Moving from #");
        Serial.print(currentPosition);
        Serial.print(" (");
        Serial.print(getFilterName(currentPosition));
        Serial.print(") to #");
        Serial.print(position);
        Serial.print(" (");
        Serial.print(getFilterName(position));
        Serial.print(") - ");
        Serial.print(stepsToMove);
        if (compensatedSteps != stepsToMove) {
            Serial.print(" steps (compensated: ");
            Serial.print(compensatedSteps);
            Serial.print(")");
        } else {
            Serial.print(" steps");
        }
        Serial.print(" = ");
        Serial.print((float)stepsToMove / getStepsPerFilter(), 2);
        Serial.print(" positions");
        if (MOTOR_DIRECTION_MODE == 0) {
            Serial.print(", UNIDIRECTIONAL");
            if (MOTOR_REVERSE_DIRECTION) {
                Serial.print(" REVERSED");
            }
        } else {
            Serial.print(", BIDIRECTIONAL");
        }
        Serial.println();

        // Additional debug for unidirectional mode
        if (MOTOR_DIRECTION_MODE == 0 && targetPosition < currentPosition) {
            Serial.print("  Path: ");
            for (int i = currentPosition + 1; i <= numFilters; i++) {
                Serial.print(getFilterName(i));
                Serial.print(" ");
            }
            for (int i = 1; i <= targetPosition; i++) {
                Serial.print(getFilterName(i));
                if (i < targetPosition) Serial.print(" ");
            }
            Serial.println();
        }
    }
}

void checkMovement() {
    if (!isMoving) return;

    // Check for timeout
    if (millis() - movementStartTime > MOVEMENT_TIMEOUT) {
        stepper.stop();
        isMoving = false;
        errorCode = ERROR_MOTOR_TIMEOUT;
        if (DEBUG_MODE) {
            Serial.println("Movement timeout!");
        }
        return;
    }

    // Update current position based on motor steps during movement
    if (isMoving) {
        long currentStep = abs(stepper.currentPosition());

        // Calculate which filter position we're currently at based on absolute steps
        uint8_t calculatedPosition = (currentStep / getStepsPerFilter()) + 1;
        if (calculatedPosition > numFilters) calculatedPosition = ((calculatedPosition - 1) % numFilters) + 1;

        // Update position if we're clearly in a new filter zone
        if (calculatedPosition != currentPosition) {
            // Calculate how far we are into this position
            long stepsFromPositionStart = currentStep % getStepsPerFilter();

            // Only update if we're well within the position (not at edges)
            if (stepsFromPositionStart > 30 || stepsFromPositionStart < (getStepsPerFilter() - 30)) {
                currentPosition = calculatedPosition;
                if (DEBUG_MODE) {
                    Serial.print("Now at filter #");
                    Serial.print(currentPosition);
                    Serial.print(" (");
                    Serial.print(getFilterName(currentPosition));
                    Serial.println(")");
                }
            }
        }
    }

    // Check if movement complete
    if (!stepper.isRunning()) {
        isMoving = false;
        currentPosition = targetPosition;

        // Save position to EEPROM when movement completes
        saveCurrentPosition();

        // Verify position with encoder if available
        if (encoderAvailable) {
            currentAngle = readAS5600Angle();
            uint8_t verifiedPosition = angleToPosition(currentAngle);

            if (verifiedPosition != currentPosition && verifiedPosition != 0) {
                // Position mismatch - try to correct
                if (DEBUG_MODE) {
                    Serial.print("Position mismatch! Expected: ");
                    Serial.print(currentPosition);
                    Serial.print(", Detected: ");
                    Serial.println(verifiedPosition);
                }
                // Could implement correction here
            }
        }

        // Schedule motor to be disabled after delay
        scheduleMotorDisable();

        if (DEBUG_MODE) {
            Serial.print("Position reached: ");
            Serial.println(currentPosition);
        }
    }
}

// ============================================
// CALIBRATION FUNCTIONS
// ============================================

void calibrateHome() {
    if (DEBUG_MODE) {
        Serial.println("Starting calibration...");
    }

    // Set current physical position as position 1
    currentPosition = 1;
    stepper.setCurrentPosition(0);

    if (encoderAvailable) {
        // Read current angle and set as reference for position 1
        float rawAngle = readAS5600Angle();
        angleOffset = -rawAngle; // This makes current angle = 0
        saveCalibration();
        if (DEBUG_MODE) {
            Serial.print("Calibration complete. Offset: ");
            Serial.println(angleOffset);
        }
    }

    // Save calibrated position to EEPROM
    saveCurrentPosition();
    isCalibrated = true;
}

// ============================================
// SERIAL COMMAND PROCESSING
// ============================================

void processCommand(String cmd) {
    cmd.trim();
    cmd.toUpperCase();

    // Remove # if present
    if (cmd.startsWith("#")) {
        cmd = cmd.substring(1);
    }

    if (DEBUG_MODE) {
        Serial.print("Command received: ");
        Serial.println(cmd);
    }

    // Get Position
    if (cmd == CMD_GET_POSITION || cmd == "GP") {
        Serial.print("P");
        Serial.println(currentPosition);
    }
    // Move to Position
    else if (cmd.startsWith(CMD_MOVE_POSITION) || cmd.startsWith("MP")) {
        uint8_t pos = cmd.charAt(2) - '0';
        if (pos >= 1 && pos <= numFilters) {
            moveToPosition(pos);
            Serial.print("M");
            Serial.println(pos);
        } else {
            Serial.println("ERROR:INVALID_POSITION");
        }
    }
    // Set Position
    else if (cmd.startsWith(CMD_SET_POSITION) || cmd.startsWith("SP")) {
        uint8_t pos = cmd.charAt(2) - '0';
        if (pos >= 1 && pos <= numFilters) {
            currentPosition = pos;
            stepper.setCurrentPosition((pos - 1) * getStepsPerFilter());
            saveCurrentPosition(); // Save new position to EEPROM
            Serial.print("S");
            Serial.println(pos);
        } else {
            Serial.println("ERROR:INVALID_POSITION");
        }
    }
    // Calibrate
    else if (cmd == CMD_CALIBRATE || cmd == "CAL") {
        calibrateHome();
        Serial.println("CALIBRATED");
    }
    // Get Status
    else if (cmd == CMD_STATUS || cmd == "STATUS") {
        Serial.print("STATUS:");
        Serial.print("POS=");
        Serial.print(currentPosition);
        Serial.print(",MOVING=");
        Serial.print(isMoving ? "YES" : "NO");
        Serial.print(",CAL=");
        Serial.print(isCalibrated ? "YES" : "NO");
        if (encoderAvailable) {
            Serial.print(",ANGLE=");
            Serial.print(currentAngle, 1);
        }
        Serial.print(",ERROR=");
        Serial.println(errorCode);
    }
    // Get Number of Filters
    else if (cmd == CMD_GET_FILTERS || cmd == "GF") {
        Serial.print("F");
        Serial.println(numFilters);
    }
    // Set Filter Count
    else if (cmd.startsWith(CMD_SET_FILTER_COUNT) || cmd.startsWith("FC")) {
        if (cmd.length() >= 3) {
            uint8_t newCount = cmd.charAt(2) - '0';
            if (newCount >= MIN_FILTER_COUNT && newCount <= MAX_FILTER_COUNT) {
            // Validate current position against new count
            if (currentPosition > newCount) {
                currentPosition = 1; // Reset to position 1 if current position is invalid
                saveCurrentPosition();
            }

            numFilters = newCount;
            saveFilterCount(); // Save to EEPROM immediately

            // Reinitialize default names for new filter positions if needed
            loadFilterNames(); // Reload names with new count

            Serial.print("FC");
            Serial.println(numFilters);

            if (DEBUG_MODE) {
                Serial.print("Filter count changed to: ");
                Serial.println(numFilters);
                Serial.print("Steps per filter: ");
                Serial.println(getStepsPerFilter());
            }
        } else {
            Serial.print("ERROR:INVALID_COUNT (range: ");
            Serial.print(MIN_FILTER_COUNT);
            Serial.print("-");
            Serial.print(MAX_FILTER_COUNT);
            Serial.println(")");
            }
        } else {
            Serial.println("ERROR:INVALID_FORMAT (use FC3-FC8)");
        }
    }
    // Get Filter Name
    else if (cmd.startsWith(CMD_GET_FILTER_NAME) || cmd.startsWith("GN")) {
        uint8_t pos = cmd.charAt(2) - '0';
        if (pos >= 1 && pos <= numFilters) {
            Serial.print("N");
            Serial.print(pos);
            Serial.print(":");
            Serial.println(getFilterName(pos));
        } else if (cmd.length() == 2) {
            // Return all filter names
            Serial.print("NAMES:");
            for (int i = 1; i <= numFilters; i++) {
                Serial.print(getFilterName(i));
                if (i < numFilters) Serial.print(",");
            }
            Serial.println();
        } else {
            Serial.println("ERROR:INVALID_FILTER");
        }
    }
    // Set Filter Name
    else if (cmd.startsWith(CMD_SET_FILTER_NAME) || cmd.startsWith("SN")) {
        // Format: SN1:Luminance or SN2:Red Filter
        if (cmd.length() > 3 && cmd.charAt(3) == ':') {
            uint8_t pos = cmd.charAt(2) - '0';
            if (pos >= 1 && pos <= numFilters) {
                String newName = cmd.substring(4); // Get name after "SN1:"
                if (newName.length() <= MAX_FILTER_NAME_LENGTH) {
                    strncpy(filterNames[pos - 1], newName.c_str(), MAX_FILTER_NAME_LENGTH);
                    filterNames[pos - 1][MAX_FILTER_NAME_LENGTH] = 0; // Ensure null termination
                    saveFilterNames(); // Save to EEPROM immediately
                    Serial.print("SN");
                    Serial.print(pos);
                    Serial.print(":");
                    Serial.println(filterNames[pos - 1]);
                } else {
                    Serial.println("ERROR:NAME_TOO_LONG");
                }
            } else {
                Serial.println("ERROR:INVALID_FILTER");
            }
        } else {
            Serial.println("ERROR:INVALID_FORMAT");
        }
    }
    // Get Version
    else if (cmd == CMD_VERSION || cmd == "VER") {
        Serial.print("VERSION:");
        Serial.println(FIRMWARE_VERSION);
    }
    // Get Device ID
    else if (cmd == CMD_DEVICE_ID || cmd == "ID") {
        Serial.print("DEVICE_ID:");
        Serial.println(DEVICE_ID);
    }
    // Emergency Stop
    else if (cmd == CMD_STOP || cmd == "STOP") {
        stepper.stop();
        isMoving = false;
        Serial.println("STOPPED");
    }
    // Step Forward
    else if (cmd.startsWith(CMD_STEP_FORWARD) || cmd.startsWith("SF")) {
        String stepsStr = cmd.substring(2);
        int steps = stepsStr.toInt();
        if (steps >= MIN_MANUAL_STEPS && steps <= MAX_MANUAL_STEPS) {
            moveSteps(steps);
            Serial.print("SF");
            Serial.println(steps);
        } else {
            Serial.print("ERROR:INVALID_STEP_COUNT (range: ");
            Serial.print(MIN_MANUAL_STEPS);
            Serial.print("-");
            Serial.print(MAX_MANUAL_STEPS);
            Serial.println(")");
        }
    }
    // Step Backward
    else if (cmd.startsWith(CMD_STEP_BACKWARD) || cmd.startsWith("SB")) {
        String stepsStr = cmd.substring(2);
        int steps = stepsStr.toInt();
        if (steps >= MIN_MANUAL_STEPS && steps <= MAX_MANUAL_STEPS) {
            moveSteps(-steps); // Negative for backward
            Serial.print("SB");
            Serial.println(steps);
        } else {
            Serial.print("ERROR:INVALID_STEP_COUNT (range: ");
            Serial.print(MIN_MANUAL_STEPS);
            Serial.print("-");
            Serial.print(MAX_MANUAL_STEPS);
            Serial.println(")");
        }
    }
    // Step To absolute position
    else if (cmd.startsWith(CMD_STEP_TO) || cmd.startsWith("ST")) {
        String stepStr = cmd.substring(2);
        long absoluteStep = stepStr.toInt();
        moveToStep(absoluteStep);
        Serial.print("ST");
        Serial.println(absoluteStep);
    }
    // Get current step position
    else if (cmd == "GST") {
        Serial.print("STEP:");
        Serial.println(getCurrentStepPosition());
    }
    // Motor Enable
    else if (cmd == CMD_MOTOR_ENABLE || cmd == "ME") {
        enableMotor();
        Serial.println("MOTOR_ENABLED");
    }
    // Motor Disable
    else if (cmd == CMD_MOTOR_DISABLE || cmd == "MD") {
        disableMotor();
        Serial.println("MOTOR_DISABLED");
    }
    // Start Revolution Calibration
    else if (cmd == CMD_START_REV_CAL || cmd == "REVCAL") {
        if (!revCalibrationMode) {
            revCalibrationMode = true;
            revCalibrationSteps = STEPS_PER_REVOLUTION;
            revCalibrationStartPosition = stepper.currentPosition();
            enableMotor();
            isMoving = true;
            movementStartTime = millis();
            stepper.move(revCalibrationSteps);
            Serial.print("REV_CAL_STARTED:");
            Serial.println(revCalibrationSteps);
            if (DEBUG_MODE) {
                Serial.println("Revolution calibration started - making full revolution");
            }
        } else {
            Serial.println("ERROR:REV_CAL_ALREADY_ACTIVE");
        }
    }
    // Revolution Calibration Adjust Plus
    else if (cmd.startsWith(CMD_REV_CAL_ADJUST_PLUS) || cmd.startsWith("RCP")) {
        if (revCalibrationMode && !isMoving) {
            String stepsStr = cmd.substring(3);
            int adjustSteps = stepsStr.toInt();
            if (adjustSteps >= 1 && adjustSteps <= 100) {
                revCalibrationSteps += adjustSteps;
                enableMotor();
                isMoving = true;
                movementStartTime = millis();
                stepper.move(adjustSteps);
                Serial.print("RCP");
                Serial.print(adjustSteps);
                Serial.print(" TOTAL:");
                Serial.println(revCalibrationSteps);
                if (DEBUG_MODE) {
                    Serial.print("Added ");
                    Serial.print(adjustSteps);
                    Serial.print(" steps. Total: ");
                    Serial.println(revCalibrationSteps);
                }
            } else {
                Serial.println("ERROR:INVALID_ADJUST_STEPS (1-100)");
            }
        } else {
            Serial.println("ERROR:NO_ACTIVE_REV_CAL_OR_MOVING");
        }
    }
    // Revolution Calibration Adjust Minus
    else if (cmd.startsWith(CMD_REV_CAL_ADJUST_MINUS) || cmd.startsWith("RCM")) {
        if (revCalibrationMode && !isMoving) {
            String stepsStr = cmd.substring(3);
            int adjustSteps = stepsStr.toInt();
            if (adjustSteps >= 1 && adjustSteps <= 100) {
                revCalibrationSteps -= adjustSteps;
                enableMotor();
                isMoving = true;
                movementStartTime = millis();
                stepper.move(-adjustSteps);
                Serial.print("RCM");
                Serial.print(adjustSteps);
                Serial.print(" TOTAL:");
                Serial.println(revCalibrationSteps);
                if (DEBUG_MODE) {
                    Serial.print("Subtracted ");
                    Serial.print(adjustSteps);
                    Serial.print(" steps. Total: ");
                    Serial.println(revCalibrationSteps);
                }
            } else {
                Serial.println("ERROR:INVALID_ADJUST_STEPS (1-100)");
            }
        } else {
            Serial.println("ERROR:NO_ACTIVE_REV_CAL_OR_MOVING");
        }
    }
    // Finish Revolution Calibration
    else if (cmd == CMD_FINISH_REV_CAL || cmd == "RCFIN") {
        if (revCalibrationMode && !isMoving) {
            calibratedStepsPerRevolution = revCalibrationSteps;
            saveRevolutionCalibration();
            revCalibrationMode = false;
            Serial.print("REV_CAL_COMPLETE:");
            Serial.println(calibratedStepsPerRevolution);
            if (DEBUG_MODE) {
                Serial.print("Revolution calibration complete. Steps per revolution: ");
                Serial.println(calibratedStepsPerRevolution);
                Serial.print("New steps per filter: ");
                Serial.println(getStepsPerFilter());
            }
        } else {
            Serial.println("ERROR:NO_ACTIVE_REV_CAL_OR_MOVING");
        }
    }
    // Start Backlash Calibration
    else if (cmd == CMD_START_BACKLASH_CAL || cmd == "BLCAL") {
        if (!backlashCalibrationMode && encoderAvailable) {
            backlashCalibrationMode = true;
            backlashTestSteps = 0;
            backlashDirectionForward = true;
            backlashTestStartPosition = stepper.currentPosition();
            Serial.println("BACKLASH_CAL_STARTED");
            if (DEBUG_MODE) {
                Serial.println("Backlash calibration started.");
                Serial.println("Use #BLS[X] to move in small steps until you detect movement with encoder");
                Serial.println("Then use #BLM to mark when movement is detected");
                Serial.println("Process will repeat for opposite direction");
            }
        } else if (!encoderAvailable) {
            Serial.println("ERROR:ENCODER_REQUIRED_FOR_BACKLASH_CAL");
        } else {
            Serial.println("ERROR:BACKLASH_CAL_ALREADY_ACTIVE");
        }
    }
    // Backlash Test Step
    else if (cmd.startsWith(CMD_BACKLASH_STEP) || cmd.startsWith("BLS")) {
        if (backlashCalibrationMode && !isMoving) {
            String stepsStr = cmd.substring(3);
            int testSteps = stepsStr.toInt();
            if (testSteps >= 1 && testSteps <= 50) {
                backlashTestSteps += testSteps;
                enableMotor();
                isMoving = true;
                movementStartTime = millis();
                int direction = backlashDirectionForward ? testSteps : -testSteps;
                stepper.move(direction);
                Serial.print("BLS");
                Serial.print(testSteps);
                Serial.print(" TOTAL:");
                Serial.println(backlashTestSteps);
                if (DEBUG_MODE) {
                    Serial.print("Test step ");
                    Serial.print(testSteps);
                    Serial.print(" (");
                    Serial.print(backlashDirectionForward ? "forward" : "backward");
                    Serial.print("). Total test steps: ");
                    Serial.println(backlashTestSteps);
                    Serial.println("Check encoder - use #BLM when movement detected");
                }
            } else {
                Serial.println("ERROR:INVALID_TEST_STEPS (1-50)");
            }
        } else {
            Serial.println("ERROR:NO_ACTIVE_BACKLASH_CAL_OR_MOVING");
        }
    }
    // Backlash Mark Movement Detected
    else if (cmd == CMD_BACKLASH_MARK || cmd == "BLM") {
        if (backlashCalibrationMode && !isMoving) {
            if (backlashDirectionForward) {
                // First direction complete, start opposite direction
                forwardBacklash = backlashTestSteps;
                backlashDirectionForward = false;
                backlashTestSteps = 0;
                Serial.print("BLM_FORWARD:");
                Serial.println(forwardBacklash);
                if (DEBUG_MODE) {
                    Serial.print("Forward backlash measured: ");
                    Serial.print(forwardBacklash);
                    Serial.println(" steps");
                    Serial.println("Now test backward direction with #BLS commands");
                }
            } else {
                // Both directions complete
                uint8_t backwardBacklash = backlashTestSteps;
                calibratedBacklashSteps = max(forwardBacklash, backwardBacklash);
                Serial.print("BLM_BACKWARD:");
                Serial.println(backwardBacklash);
                Serial.print("BLM_COMPLETE:");
                Serial.println(calibratedBacklashSteps);
                if (DEBUG_MODE) {
                    Serial.print("Backward backlash measured: ");
                    Serial.print(backwardBacklash);
                    Serial.println(" steps");
                    Serial.print("Using maximum backlash: ");
                    Serial.print(calibratedBacklashSteps);
                    Serial.println(" steps");
                    Serial.println("Use #BLFIN to save calibration");
                }
            }
        } else {
            Serial.println("ERROR:NO_ACTIVE_BACKLASH_CAL_OR_MOVING");
        }
    }
    // Finish Backlash Calibration
    else if (cmd == CMD_FINISH_BACKLASH_CAL || cmd == "BLFIN") {
        if (backlashCalibrationMode && !isMoving && !backlashDirectionForward) {
            saveBacklashCalibration();
            backlashCalibrationMode = false;
            Serial.print("BACKLASH_CAL_COMPLETE:");
            Serial.println(calibratedBacklashSteps);
            if (DEBUG_MODE) {
                Serial.print("Backlash calibration saved: ");
                Serial.print(calibratedBacklashSteps);
                Serial.println(" steps");
            }
        } else {
            Serial.println("ERROR:BACKLASH_CAL_NOT_READY");
        }
    }
    else {
        Serial.println("ERROR:UNKNOWN_COMMAND");
    }
}

void handleSerial() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (commandBuffer.length() > 0) {
                processCommand(commandBuffer);
                commandBuffer = "";
            }
        } else {
            commandBuffer += c;

            // Prevent buffer overflow
            if (commandBuffer.length() > 50) {
                commandBuffer = "";
                if (DEBUG_MODE) {
                    Serial.println("ERROR:BUFFER_OVERFLOW");
                }
            }
        }
    }
}

// ============================================
// BUTTON HANDLING (OPTIONAL)
// ============================================

void handleButtons() {
    static unsigned long lastButtonPress = 0;
    const unsigned long DEBOUNCE_DELAY = 200;

    if (millis() - lastButtonPress < DEBOUNCE_DELAY) return;

    // Check next button
    if (digitalRead(BUTTON_NEXT) == LOW) {
        lastButtonPress = millis();
        uint8_t nextPos = currentPosition + 1;
        if (nextPos > numFilters) nextPos = 1;
        moveToPosition(nextPos);
    }

    // Check previous button - only works in bidirectional mode
    if (digitalRead(BUTTON_PREV) == LOW) {
        lastButtonPress = millis();

        if (MOTOR_DIRECTION_MODE == 1) {
            // BIDIRECTIONAL MODE - Allow backward movement
            uint8_t prevPos = currentPosition - 1;
            if (prevPos < 1) prevPos = numFilters;
            moveToPosition(prevPos);
        } else {
            // UNIDIRECTIONAL MODE - Go forward to previous position
            // Example: from 2 to 1, or from 1 to 5
            uint8_t prevPos = currentPosition - 1;
            if (prevPos < 1) prevPos = numFilters;
            moveToPosition(prevPos);  // Will automatically go forward through all positions

            if (DEBUG_MODE) {
                Serial.println("Note: In unidirectional mode, going to previous filter via forward movement");
            }
        }
    }
}

// ============================================
// SETUP
// ============================================

void setup() {
    // Initialize Serial
    Serial.begin(SERIAL_BAUD_RATE);

    if (DEBUG_MODE) {
        Serial.println("\n\n========================================");
        Serial.println(DEVICE_NAME);
        Serial.print("Version: ");
        Serial.println(FIRMWARE_VERSION);
        Serial.print("Filters: ");
        Serial.println(numFilters);
        Serial.print("Direction Mode: ");
        if (MOTOR_DIRECTION_MODE == 0) {
            Serial.print("UNIDIRECTIONAL");
            if (MOTOR_REVERSE_DIRECTION) {
                Serial.print(" (REVERSED)");
            }
        } else {
            Serial.print("BIDIRECTIONAL");
        }
        Serial.println();
        Serial.println("========================================\n");
    }

    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        if (DEBUG_MODE) {
            Serial.println("SSD1306 allocation failed");
        }
    } else {
        // Set contrast to maximum for better visibility on small display
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(255);

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(OLED_X_OFFSET + 5, OLED_Y_OFFSET + 10);
        display.print("FILTER");
        display.setCursor(OLED_X_OFFSET + 5, OLED_Y_OFFSET + 20);
        display.print("WHEEL");
        display.display();
        delay(1500);
    }

    // Initialize AS5600 encoder
    initAS5600();

    // Calculate filter angles
    for (uint8_t i = 0; i < numFilters; i++) {
        filterAngles[i] = (360.0 / numFilters) * i;
    }

    // Initialize stepper motor
    stepper.setMaxSpeed(MAX_MOTOR_SPEED);
    stepper.setAcceleration(MOTOR_ACCELERATION);
    stepper.setSpeed(MOTOR_SPEED);

    // Configure motor pins as outputs
    pinMode(MOTOR_PIN1, OUTPUT);
    pinMode(MOTOR_PIN2, OUTPUT);
    pinMode(MOTOR_PIN3, OUTPUT);
    pinMode(MOTOR_PIN4, OUTPUT);

    // Force all pins LOW initially
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);

    if (DEBUG_MODE) {
        Serial.print("Motor pins configured: ");
        Serial.print(MOTOR_PIN1);
        Serial.print(",");
        Serial.print(MOTOR_PIN2);
        Serial.print(",");
        Serial.print(MOTOR_PIN3);
        Serial.print(",");
        Serial.println(MOTOR_PIN4);
    }

    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_NEXT, INPUT_PULLUP);
    pinMode(BUTTON_PREV, INPUT_PULLUP);

    // Initialize motor enable pin if configured
    if (MOTOR_ENABLE_PIN >= 0) {
        pinMode(MOTOR_ENABLE_PIN, OUTPUT);
        digitalWrite(MOTOR_ENABLE_PIN, HIGH); // Enable motor initially
    }

    // Load calibration from EEPROM
    loadCalibration();

    // Load filter count from EEPROM (must be loaded before position and names)
    loadFilterCount();

    // Load revolution calibration from EEPROM
    loadRevolutionCalibration();

    // Load backlash calibration from EEPROM
    loadBacklashCalibration();

    // Load saved position from EEPROM
    loadCurrentPosition();

    // Load filter names from EEPROM
    loadFilterNames();

    // Initial position reading
    if (encoderAvailable) {
        currentAngle = readAS5600Angle();
        uint8_t detectedPosition = angleToPosition(currentAngle);
        if (detectedPosition > 0 && isCalibrated) {
            currentPosition = detectedPosition;
            if (DEBUG_MODE) {
                Serial.print("Detected position: ");
                Serial.println(currentPosition);
            }
        }
    }

    if (DEBUG_MODE) {
        // Ready message
        // Display filter configuration
        Serial.println("\nFilter Configuration:");
        for (int i = 1; i <= numFilters; i++) {
            Serial.print("  Position ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(getFilterName(i));
        }

        Serial.println("\nSystem ready!");
        Serial.println("Commands:");
        Serial.println("  #GP - Get position");
        Serial.println("  #MP[1-5] - Move to position");
        Serial.println("  #SP[1-5] - Set current position");
        Serial.println("  #GF - Get number of filters");
        Serial.println("  #FC[3-8] - Set filter count (e.g. #FC4 for 4 filters)");
        Serial.println("  #GN - Get all filter names");
        Serial.println("  #GN[1-X] - Get specific filter name");
        Serial.println("  #SN[1-X]:Name - Set filter name (e.g. #SN1:Luminance)");
        Serial.println("  #CAL - Calibrate home");
        Serial.println("  #STATUS - Get status");
        Serial.println("  #ID - Get device identifier");
        Serial.println("  #VER - Get firmware version");
        Serial.println("  #STOP - Emergency stop");
        Serial.println("Manual stepping:");
        Serial.println("  #SF[X] - Step forward X steps (e.g. #SF100)");
        Serial.println("  #SB[X] - Step backward X steps (e.g. #SB50)");
        Serial.println("  #ST[X] - Step to absolute position (e.g. #ST1024)");
        Serial.println("  #GST - Get current step position");
        Serial.println("Motor power:");
        Serial.println("  #ME - Enable motor power");
        Serial.println("  #MD - Disable motor power (power saving)");
        Serial.println();
    }
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
    // Handle serial commands
    handleSerial();

    // Update motor movement ONLY if motor is enabled
    if (motorEnabled) {
        stepper.run();
    }
    checkMovement();

    // Read encoder if available
    if (encoderAvailable && !isMoving) {
        currentAngle = readAS5600Angle();
    }

    // Handle manual buttons
    handleButtons();

    // Handle motor power management
    handleMotorPowerManagement();

    // Update display
    updateDisplay();

    // Update status LED (blink when motor disabled)
    if (motorEnabled) {
        digitalWrite(LED_PIN, isMoving ? HIGH : LOW);
    } else {
        // Slow blink when motor is disabled
        digitalWrite(LED_PIN, (millis() / 1000) % 2);
    }
}