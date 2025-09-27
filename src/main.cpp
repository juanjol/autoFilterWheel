#include <Arduino.h>
#include "core/FilterWheelController.h"
#include "config.h"

// ============================================
// MAIN FILTER WHEEL CONTROLLER
// ============================================

FilterWheelController controller;

// ============================================
// ARDUINO SETUP
// ============================================

void setup() {
    // Initialize serial for debugging and commands
    Serial.begin(115200);
    Serial.println();
    Serial.println("========================================");
    Serial.println("ESP32-C3 Filter Wheel Controller v1.0.0");
    Serial.println("========================================");

    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize controller with ULN2003 motor driver
    if (!controller.init(MotorDriverType::ULN2003_28BYJ48)) {
        Serial.println("ERROR: Failed to initialize filter wheel controller!");
        Serial.println("Check hardware connections and restart.");
        while(1) {
            delay(1000);
        }
    }

    Serial.println("Filter wheel controller initialized successfully.");
    Serial.println("System ready for commands.");
    Serial.println("Type #HELP for available commands.");
    Serial.println();

    // Show system status
    Serial.println(controller.getSystemStatus());
    Serial.println();
}

// ============================================
// ARDUINO MAIN LOOP
// ============================================

void loop() {
    // Update controller (handles motor movement, display, power management)
    controller.update();

    // Handle serial commands
    controller.handleSerial();

    // Small delay to prevent excessive CPU usage
    delay(1);
}

// ============================================
// HARDWARE INTERRUPT HANDLERS (if needed)
// ============================================

// Add any hardware interrupt handlers here if required for future features
// For example, physical buttons, emergency stop switches, etc.