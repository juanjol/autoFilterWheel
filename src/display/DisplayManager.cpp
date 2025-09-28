#include "DisplayManager.h"
#include "../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

DisplayManager::DisplayManager(uint8_t width, uint8_t height, TwoWire* wire,
                               int8_t resetPin, uint8_t xOffset)
    : display(nullptr)
    , screenWidth(width)
    , screenHeight(height)
    , resetPin(resetPin)
    , xOffset(xOffset)
    , lastUpdate(0)
    , updateInterval(100)  // 100ms default
    , displayEnabled(true)
    , needsUpdate(false)
    , rotation180(OLED_ROTATION_180)  // Use default from config
{
    display = new Adafruit_SSD1306(width, height, wire, resetPin);
}

bool DisplayManager::init(uint8_t address) {
    if (!display) {
        return false;
    }

    if (!display->begin(SSD1306_SWITCHCAPVCC, address)) {
        return false;
    }

    // Load display configuration from EEPROM
    loadDisplayConfig();

    // Configure display
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setTextWrap(false);

    // Set rotation
    display->setRotation(rotation180 ? 2 : 0);  // 0 = normal, 2 = 180 degrees

    showSplashScreen();
    forceUpdate();

    return true;
}

void DisplayManager::update() {
    if (!displayEnabled || !needsUpdate) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= updateInterval) {
        performUpdate();
        lastUpdate = currentTime;
        needsUpdate = false;
    }
}

void DisplayManager::forceUpdate() {
    if (!displayEnabled) {
        return;
    }

    performUpdate();
    lastUpdate = millis();
    needsUpdate = false;
}

void DisplayManager::setEnabled(bool enabled) {
    displayEnabled = enabled;
    if (!enabled) {
        clear();
        forceUpdate();
    }
}

bool DisplayManager::isEnabled() const {
    return displayEnabled;
}

void DisplayManager::setUpdateInterval(uint16_t intervalMs) {
    updateInterval = intervalMs;
}

void DisplayManager::showStatus(const char* status) {
    if (!display) return;

    display->setTextSize(1);
    display->setCursor(xOffset, STATUS_LINE_Y);
    display->print(status);
    needsUpdate = true;
}

void DisplayManager::showPosition(uint8_t position, uint8_t maxPosition) {
    if (!display) return;

    display->setTextSize(2);  // Large text for position

    char posText[16];
    snprintf(posText, sizeof(posText), "POS %d", position);

    uint8_t x = centerTextX(posText, 2);
    display->setCursor(x, POSITION_LINE_Y);
    display->print(posText);
    needsUpdate = true;
}

void DisplayManager::showFilterName(const char* filterName) {
    if (!display) return;

    display->setTextSize(1);

    // Truncate filter name if too long
    char truncatedName[16];
    truncateText(truncatedName, filterName, 12);

    uint8_t x = centerTextX(truncatedName, 1);
    display->setCursor(x, FILTER_NAME_LINE_Y);
    display->print(truncatedName);
    needsUpdate = true;
}

void DisplayManager::showFilterWheelState(const char* status, uint8_t position,
                                          uint8_t maxPosition, const char* filterName,
                                          bool isMoving) {
    if (!display) return;

    display->clearDisplay();

    // Status line
    display->setTextSize(1);
    const char* displayStatus = isMoving ? "MOVING" : status;
    drawCenteredText(displayStatus, STATUS_LINE_Y, 1);

    // Position line (large text)
    display->setTextSize(2);
    char posText[16];
    snprintf(posText, sizeof(posText), "POS %d", position);
    drawCenteredText(posText, POSITION_LINE_Y, 2);

    // Filter name line
    display->setTextSize(1);
    char truncatedName[16];
    truncateText(truncatedName, filterName, 12);
    drawCenteredText(truncatedName, FILTER_NAME_LINE_Y, 1);

    // Immediately update the display
    display->display();
    needsUpdate = true;
}

void DisplayManager::showCalibrationProgress(uint8_t step, uint8_t totalSteps, const char* message) {
    if (!display) return;

    display->clearDisplay();

    // Calibration title
    drawCenteredText("CALIBRATION", STATUS_LINE_Y, 1);

    // Progress
    char progressText[16];
    snprintf(progressText, sizeof(progressText), "Step %d/%d", step, totalSteps);
    drawCenteredText(progressText, POSITION_LINE_Y, 1);

    // Message
    char truncatedMsg[16];
    truncateText(truncatedMsg, message, 12);
    drawCenteredText(truncatedMsg, FILTER_NAME_LINE_Y, 1);

    needsUpdate = true;
}

void DisplayManager::showError(uint8_t errorCode, const char* errorMessage) {
    if (!display) return;

    display->clearDisplay();

    // Error indicator
    drawCenteredText("ERROR", STATUS_LINE_Y, 1);

    // Error code
    char errorText[16];
    snprintf(errorText, sizeof(errorText), "Code: %d", errorCode);
    drawCenteredText(errorText, POSITION_LINE_Y, 1);

    // Error message
    char truncatedMsg[16];
    truncateText(truncatedMsg, errorMessage, 12);
    drawCenteredText(truncatedMsg, FILTER_NAME_LINE_Y, 1);

    needsUpdate = true;
}

void DisplayManager::showConfigMenu(const char* menuItem, const char* value) {
    if (!display) return;

    display->clearDisplay();

    // Menu title
    drawCenteredText("CONFIG", STATUS_LINE_Y, 1);

    // Menu item
    char truncatedItem[16];
    truncateText(truncatedItem, menuItem, 12);
    drawCenteredText(truncatedItem, POSITION_LINE_Y, 1);

    // Value
    char truncatedValue[16];
    truncateText(truncatedValue, value, 12);
    drawCenteredText(truncatedValue, FILTER_NAME_LINE_Y, 1);

    needsUpdate = true;
}

void DisplayManager::clear() {
    if (!display) return;
    display->clearDisplay();
    needsUpdate = true;
}

void DisplayManager::showSplashScreen() {
    if (!display) return;

    display->clearDisplay();

    // Title
    drawCenteredText("ESP32-C3", STATUS_LINE_Y, 1);
    drawCenteredText("Filter", POSITION_LINE_Y, 1);
    drawCenteredText("Wheel", FILTER_NAME_LINE_Y, 1);

    needsUpdate = true;
}

void DisplayManager::showVersionInfo(const char* version, const char* driver) {
    if (!display) return;

    display->clearDisplay();

    // Version
    char versionText[16];
    snprintf(versionText, sizeof(versionText), "v%s", version);
    drawCenteredText(versionText, STATUS_LINE_Y, 1);

    // Driver type
    char truncatedDriver[16];
    truncateText(truncatedDriver, driver, 12);
    drawCenteredText(truncatedDriver, POSITION_LINE_Y, 1);

    drawCenteredText("Ready", FILTER_NAME_LINE_Y, 1);

    needsUpdate = true;
}

void DisplayManager::runDisplayTest() {
    if (!display) return;

    // Test pattern 1: All pixels
    display->clearDisplay();
    for (int16_t i = 0; i < screenWidth; i += 4) {
        for (int16_t j = 0; j < screenHeight; j += 4) {
            display->drawPixel(i, j, SSD1306_WHITE);
        }
    }
    forceUpdate();
    delay(1000);

    // Test pattern 2: Text at different positions
    for (uint8_t i = 0; i < 3; i++) {
        display->clearDisplay();
        char testText[16];
        snprintf(testText, sizeof(testText), "Test %d", i + 1);
        drawCenteredText(testText, STATUS_LINE_Y + (i * 12), 1);
        forceUpdate();
        delay(500);
    }

    clear();
    forceUpdate();
}

void DisplayManager::performUpdate() {
    if (display) {
        display->display();
    }
}

uint8_t DisplayManager::centerTextX(const char* text, uint8_t textSize) {
    uint8_t textWidth = strlen(text) * 6 * textSize;  // Approximation
    if (textWidth >= (screenWidth - xOffset)) {
        return xOffset;
    }
    return xOffset + ((screenWidth - xOffset - textWidth) / 2);
}

void DisplayManager::drawCenteredText(const char* text, uint8_t y, uint8_t textSize) {
    if (!display) return;

    display->setTextSize(textSize);
    uint8_t x = centerTextX(text, textSize);
    display->setCursor(x, y);
    display->print(text);
}

void DisplayManager::truncateText(char* buffer, const char* text, uint8_t maxChars) {
    uint8_t len = strlen(text);
    if (len <= maxChars) {
        strcpy(buffer, text);
    } else {
        strncpy(buffer, text, maxChars - 3);
        buffer[maxChars - 3] = '.';
        buffer[maxChars - 2] = '.';
        buffer[maxChars - 1] = '.';
        buffer[maxChars] = '\0';
    }
}

void DisplayManager::setRotation(bool rotate180) {
    rotation180 = rotate180;

    if (display) {
        display->setRotation(rotation180 ? 2 : 0);  // 0 = normal, 2 = 180 degrees
        needsUpdate = true;
        forceUpdate();  // Immediate update to show rotation change
    }

    // Save to EEPROM
    saveDisplayConfig();

    Serial.print("Display rotation: ");
    Serial.println(rotation180 ? "180°" : "Normal");
}

void DisplayManager::saveDisplayConfig() {
    EEPROM.write(EEPROM_DISPLAY_CONFIG_FLAG, 0xAA);  // Magic byte to indicate config is saved
    EEPROM.write(EEPROM_DISPLAY_ROTATION, rotation180 ? 1 : 0);
    EEPROM.commit();
}

void DisplayManager::loadDisplayConfig() {
    // Check if display config is saved in EEPROM
    if (EEPROM.read(EEPROM_DISPLAY_CONFIG_FLAG) == 0xAA) {
        uint8_t rotationValue = EEPROM.read(EEPROM_DISPLAY_ROTATION);
        rotation180 = (rotationValue == 1);
        Serial.println("Display configuration loaded from EEPROM");
    } else {
        // Use default from config.h
        rotation180 = OLED_ROTATION_180;
        Serial.println("Using default display configuration");
    }
}