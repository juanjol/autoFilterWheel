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
    , displayOn(true)
    , needsUpdate(false)
    , rotation180(OLED_ROTATION_180)  // Use default from config
    , displayMode(DEFAULT_DISPLAY_MODE)  // Use default display mode from config
    , brightness(255)  // Default to maximum brightness
    , powerMode(DISPLAY_POWER_MODE)
    , autoOffTimeout(DISPLAY_AUTO_OFF_TIMEOUT)
    , lastActivityTime(0)
    , displayOffTime(0)
    , scrollOffset(0)
    , lastScrollTime(0)
    , scrollDelay(300)  // 300ms between scroll steps
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

    // Set brightness/contrast
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(brightness);

    showSplashScreen();
    forceUpdate();

    return true;
}

void DisplayManager::update() {
    unsigned long currentTime = millis();

    // Handle auto-off logic
    if (powerMode == DISPLAY_POWER_MODE_AUTO && displayOn && autoOffTimeout > 0) {
        if (displayOffTime > 0 && currentTime >= displayOffTime) {
            turnOff();
        }
    }

    if (!displayEnabled || !needsUpdate) {
        return;
    }

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
    display->setCursor(xOffset, getStatusLineY());
    display->print(status);
    needsUpdate = true;
}

void DisplayManager::showPosition(uint8_t position, uint8_t maxPosition) {
    if (!display) return;

    display->setTextSize(2);  // Large text for position

    char posText[16];
    snprintf(posText, sizeof(posText), "POS %d", position);

    uint8_t x = centerTextX(posText, 2);
    display->setCursor(x, getPositionLineY());
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
    display->setCursor(x, getFilterNameLineY());
    display->print(truncatedName);
    needsUpdate = true;
}

void DisplayManager::showFilterWheelState(const char* status, uint8_t position,
                                          uint8_t maxPosition, const char* filterName,
                                          bool isMoving) {
    if (!display) return;

    resetActivityTimer();  // Reset timer on any display activity

    // Choose display mode
    if (displayMode == DISPLAY_MODE_MINIMAL) {
        // Use minimal display with large number
        showFilterWheelStateMinimal(position, maxPosition, filterName, isMoving);
        return;
    }

    // DISPLAY_MODE_DETAILED: Original 3-line display
    // Ensure horizontal orientation for detailed mode
    display->setRotation(rotation180 ? 2 : 0);  // 0=normal, 2=180°

    display->clearDisplay();

    // Status line
    display->setTextSize(1);
    const char* displayStatus = isMoving ? "MOVING" : status;
    drawCenteredText(displayStatus, getStatusLineY(), 1);

    // Position line (large text)
    display->setTextSize(2);
    char posText[16];
    snprintf(posText, sizeof(posText), "POS %d", position);
    drawCenteredText(posText, getPositionLineY(), 2);

    // Filter name line
    display->setTextSize(1);
    char truncatedName[16];
    truncateText(truncatedName, filterName, 12);
    drawCenteredText(truncatedName, getFilterNameLineY(), 1);

    // Update display (performUpdate handles displayOn state)
    if (displayOn) {
        display->display();
    } else {
        display->clearDisplay();
        display->display();
    }
    needsUpdate = true;
}

void DisplayManager::showFilterWheelStateMinimal(uint8_t position, uint8_t totalFilters, const char* filterName, bool moving) {
    if (!display) return;

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);

    // Set vertical orientation (90° rotation)
    // Use rotation(3) by default for proper vertical orientation
    // rotation180 flag inverts it to rotation(1)
    display->setRotation(rotation180 ? 1 : 3);

    // After rotation(3): dimensions are 64px wide x 128px tall
    // For 0.42" OLED: visible area is approximately 40px wide x 72px tall
    // Using defined constants for consistent visible area across rotations

    // Get visible area from constants
    const uint8_t visibleX = rotation180 ? VERTICAL_X_OFFSET_ROTATED : VERTICAL_X_OFFSET_NORMAL;
    const uint8_t visibleY = rotation180 ? VERTICAL_Y_OFFSET_ROTATED : VERTICAL_Y_OFFSET_NORMAL;
    const uint8_t visibleWidth = VERTICAL_VISIBLE_WIDTH;
    const uint8_t visibleHeight = VERTICAL_VISIBLE_HEIGHT;

    // Draw large position number
    display->setTextSize(5);  // Very large text for position number

    char numStr[3];
    snprintf(numStr, sizeof(numStr), "%d", position);

    // Character size in textSize(5): 30 pixels wide x 40 pixels tall
    uint8_t numCharWidth = 30;
    uint8_t numCharHeight = 40;
    uint8_t numWidth = strlen(numStr) * numCharWidth;

    // Position number near left edge with small margin
    uint8_t numX = visibleX + 3;  // 3px margin from left edge
    uint8_t numY = visibleY + 5;  // Near top of visible area

    display->setCursor(numX, numY);
    display->print(position);

    // Draw filter name with horizontal scrolling for long names
    display->setTextSize(1);  // Small text for filter name

    // Check if name is too long and needs scrolling
    uint8_t nameCharWidth = 6;
    uint8_t maxVisibleChars = 6;  // Force scroll for names longer than 6 chars
    uint8_t nameLen = strlen(filterName);

    // Position filter name below the number, aligned to left
    uint8_t nameX = visibleX + 3;  // Align with number (3px margin)
    uint8_t nameY = numY + numCharHeight + 5;  // Below number with 5px gap

    if (nameLen <= maxVisibleChars) {
        // Name fits, display normally without scrolling
        scrollOffset = 0;  // Reset scroll
        display->setCursor(nameX, nameY);
        display->print(filterName);
    } else {
        // Name is too long, implement continuous marquee scrolling
        unsigned long currentTime = millis();

        // Update scroll position periodically
        if (currentTime - lastScrollTime >= scrollDelay) {
            scrollOffset++;

            // Create continuous loop: text + 3 spaces + text repeats
            // Reset when we've scrolled one full cycle (name length + 3 spaces)
            if (scrollOffset >= (int16_t)(nameLen + 3)) {
                scrollOffset = 0;
            }

            lastScrollTime = currentTime;
        }

        // Build continuous scrolling text: "name   name   name..."
        // where "   " is 3 spaces between repetitions
        char scrolledName[32];

        for (int i = 0; i < maxVisibleChars; i++) {
            // Calculate position in the infinite repeating sequence
            int16_t pos = (scrollOffset + i) % (nameLen + 3);

            if (pos < nameLen) {
                // Within the name
                scrolledName[i] = filterName[pos];
            } else {
                // In the gap (3 spaces)
                scrolledName[i] = ' ';
            }
        }
        scrolledName[maxVisibleChars] = '\0';

        display->setCursor(nameX, nameY);
        display->print(scrolledName);
    }

    // Show movement indicator if moving
    if (moving) {
        // Small dot indicator near bottom of visible area
        display->fillCircle(visibleX + visibleWidth / 2, visibleY + visibleHeight - 5, 2, SSD1306_WHITE);
    }

    // Update display (check displayOn state)
    if (displayOn) {
        display->display();
    } else {
        display->clearDisplay();
        display->display();
    }

    // Mark for continuous update if scrolling is active
    if (nameLen > maxVisibleChars) {
        needsUpdate = true;
    }
}

void DisplayManager::showCalibrationProgress(uint8_t step, uint8_t totalSteps, const char* message) {
    if (!display) return;

    display->clearDisplay();

    // Calibration title
    drawCenteredText("CALIBRATION", getStatusLineY(), 1);

    // Progress
    char progressText[16];
    snprintf(progressText, sizeof(progressText), "Step %d/%d", step, totalSteps);
    drawCenteredText(progressText, getPositionLineY(), 1);

    // Message
    char truncatedMsg[16];
    truncateText(truncatedMsg, message, 12);
    drawCenteredText(truncatedMsg, getFilterNameLineY(), 1);

    needsUpdate = true;
}

void DisplayManager::showError(uint8_t errorCode, const char* errorMessage) {
    if (!display) return;

    display->clearDisplay();

    // Error indicator
    drawCenteredText("ERROR", getStatusLineY(), 1);

    // Error code
    char errorText[16];
    snprintf(errorText, sizeof(errorText), "Code: %d", errorCode);
    drawCenteredText(errorText, getPositionLineY(), 1);

    // Error message
    char truncatedMsg[16];
    truncateText(truncatedMsg, errorMessage, 12);
    drawCenteredText(truncatedMsg, getFilterNameLineY(), 1);

    needsUpdate = true;
}

void DisplayManager::showConfigMenu(const char* menuItem, const char* value) {
    if (!display) return;

    display->clearDisplay();

    // Menu title
    drawCenteredText("CONFIG", getStatusLineY(), 1);

    // Menu item
    char truncatedItem[16];
    truncateText(truncatedItem, menuItem, 12);
    drawCenteredText(truncatedItem, getPositionLineY(), 1);

    // Value
    char truncatedValue[16];
    truncateText(truncatedValue, value, 12);
    drawCenteredText(truncatedValue, getFilterNameLineY(), 1);

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
    drawCenteredText("ESP32-C3", getStatusLineY(), 1);
    drawCenteredText("Filter", getPositionLineY(), 1);
    drawCenteredText("Wheel", getFilterNameLineY(), 1);

    needsUpdate = true;
}

void DisplayManager::showVersionInfo(const char* version, const char* driver) {
    if (!display) return;

    display->clearDisplay();

    // Version
    char versionText[16];
    snprintf(versionText, sizeof(versionText), "v%s", version);
    drawCenteredText(versionText, getStatusLineY(), 1);

    // Driver type
    char truncatedDriver[16];
    truncateText(truncatedDriver, driver, 12);
    drawCenteredText(truncatedDriver, getPositionLineY(), 1);

    drawCenteredText("Ready", getFilterNameLineY(), 1);

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
        drawCenteredText(testText, getStatusLineY() + (i * 12), 1);
        forceUpdate();
        delay(500);
    }

    clear();
    forceUpdate();
}

void DisplayManager::performUpdate() {
    if (!display) return;

    // If display is off, show blank screen (OLED pixels off = power saving)
    if (!displayOn) {
        display->clearDisplay();
    }

    display->display();
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

void DisplayManager::setDisplayMode(uint8_t mode) {
    if (mode > DISPLAY_MODE_DETAILED) {
        mode = DISPLAY_MODE_MINIMAL;  // Default to minimal if invalid
    }

    displayMode = mode;
    needsUpdate = true;
    forceUpdate();  // Immediate update to show mode change

    // Save to EEPROM
    saveDisplayConfig();

    Serial.print("Display mode: ");
    Serial.println(mode == DISPLAY_MODE_MINIMAL ? "Minimal (Large Number)" : "Detailed (Full Info)");
}

void DisplayManager::setBrightness(uint8_t newBrightness) {
    brightness = newBrightness;

    if (display) {
        // Set OLED contrast (brightness)
        display->ssd1306_command(SSD1306_SETCONTRAST);
        display->ssd1306_command(brightness);

        // Force a display update to apply the change immediately
        needsUpdate = true;
        forceUpdate();
    }

    // Save to EEPROM
    saveDisplayConfig();

    Serial.print("Display brightness set to: ");
    Serial.print(brightness);
    Serial.println("/255");
    Serial.println("Note: OLED brightness range may be limited. Try values: 0, 64, 128, 192, 255");
}

void DisplayManager::saveDisplayConfig() {
    EEPROM.write(EEPROM_DISPLAY_CONFIG_FLAG, 0xAA);  // Magic byte to indicate config is saved
    EEPROM.write(EEPROM_DISPLAY_ROTATION, rotation180 ? 1 : 0);
    EEPROM.write(EEPROM_DISPLAY_MODE, displayMode);
    EEPROM.write(EEPROM_DISPLAY_BRIGHTNESS, brightness);
    EEPROM.write(EEPROM_DISPLAY_POWER_MODE, powerMode);
    EEPROM.write(EEPROM_DISPLAY_TIMEOUT, autoOffTimeout & 0xFF);
    EEPROM.write(EEPROM_DISPLAY_TIMEOUT + 1, (autoOffTimeout >> 8) & 0xFF);
    EEPROM.commit();
}

void DisplayManager::loadDisplayConfig() {
    // Check if display config is saved in EEPROM
    if (EEPROM.read(EEPROM_DISPLAY_CONFIG_FLAG) == 0xAA) {
        uint8_t rotationValue = EEPROM.read(EEPROM_DISPLAY_ROTATION);
        rotation180 = (rotationValue == 1);
        displayMode = EEPROM.read(EEPROM_DISPLAY_MODE);
        brightness = EEPROM.read(EEPROM_DISPLAY_BRIGHTNESS);
        if (brightness == 0xFF) brightness = 255;  // Default if not set
        powerMode = EEPROM.read(EEPROM_DISPLAY_POWER_MODE);
        autoOffTimeout = EEPROM.read(EEPROM_DISPLAY_TIMEOUT) | (EEPROM.read(EEPROM_DISPLAY_TIMEOUT + 1) << 8);
        Serial.println("Display configuration loaded from EEPROM");
    } else {
        // Use default from config.h
        rotation180 = OLED_ROTATION_180;
        displayMode = DEFAULT_DISPLAY_MODE;
        brightness = 255;  // Default to maximum brightness
        powerMode = DISPLAY_POWER_MODE;
        autoOffTimeout = DISPLAY_AUTO_OFF_TIMEOUT;
        Serial.println("Using default display configuration");
    }

    // Apply power mode on startup
    if (powerMode == DISPLAY_POWER_MODE_ALWAYS_OFF) {
        turnOff();
    } else {
        turnOn();
        resetActivityTimer();
    }
}

void DisplayManager::turnOn() {
    if (!display) return;

    displayOn = true;
    display->ssd1306_command(SSD1306_DISPLAYON);
    resetActivityTimer();

    Serial.println("Display turned ON");
}

void DisplayManager::turnOff() {
    if (!display) return;

    displayOn = false;
    display->ssd1306_command(SSD1306_DISPLAYOFF);

    Serial.println("Display turned OFF");
}

void DisplayManager::setPowerMode(uint8_t mode) {
    if (mode > DISPLAY_POWER_MODE_ALWAYS_OFF) {
        mode = DISPLAY_POWER_MODE_AUTO;
    }

    powerMode = mode;

    // Apply mode immediately
    switch (powerMode) {
        case DISPLAY_POWER_MODE_ALWAYS_ON:
            turnOn();
            break;
        case DISPLAY_POWER_MODE_ALWAYS_OFF:
            turnOff();
            break;
        case DISPLAY_POWER_MODE_AUTO:
            turnOn();
            resetActivityTimer();
            break;
    }

    saveDisplayConfig();

    Serial.print("Display power mode: ");
    const char* modeStr[] = {"Auto", "Always On", "Always Off"};
    Serial.println(modeStr[powerMode]);
}

void DisplayManager::setAutoOffTimeout(uint16_t seconds) {
    autoOffTimeout = seconds;
    resetActivityTimer();
    saveDisplayConfig();

    Serial.print("Display auto-off timeout: ");
    Serial.print(autoOffTimeout);
    Serial.println(" seconds");
}

void DisplayManager::resetActivityTimer() {
    lastActivityTime = millis();

    // Calculate when display should turn off
    if (autoOffTimeout > 0) {
        displayOffTime = lastActivityTime + (autoOffTimeout * 1000UL);
    } else {
        displayOffTime = 0;  // Never turn off
    }

    // Turn on display if in auto mode and currently off
    if (powerMode == DISPLAY_POWER_MODE_AUTO && !displayOn) {
        turnOn();
    }
}

void DisplayManager::showTransition(uint8_t fromPosition, uint8_t toPosition) {
    if (!display) return;

    resetActivityTimer();  // Keep display on during transition

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);

    // Set vertical orientation (90° rotation)
    display->setRotation(rotation180 ? 1 : 3);

    // Get visible area from constants for consistency
    const uint8_t visibleX = rotation180 ? VERTICAL_X_OFFSET_ROTATED : VERTICAL_X_OFFSET_NORMAL;
    const uint8_t visibleY = rotation180 ? VERTICAL_Y_OFFSET_ROTATED : VERTICAL_Y_OFFSET_NORMAL;
    const uint8_t visibleWidth = VERTICAL_VISIBLE_WIDTH;
    const uint8_t visibleHeight = VERTICAL_VISIBLE_HEIGHT;

    // Show "FROM → TO" with smaller text to fit everything
    display->setTextSize(3);  // Medium text for numbers (18px wide x 24px tall)

    // From position - near top
    uint8_t fromX = visibleX + 10;
    uint8_t fromY = visibleY + 2;
    display->setCursor(fromX, fromY);
    display->print(fromPosition);

    // Arrow - centered vertically
    display->setTextSize(2);  // 12px wide x 16px tall
    uint8_t arrowX = visibleX + 10;
    uint8_t arrowY = fromY + 26;  // Below from number
    display->setCursor(arrowX, arrowY);
    display->print("->");

    // To position - below arrow
    display->setTextSize(3);  // Same size as from number
    uint8_t toX = visibleX + 10;
    uint8_t toY = arrowY + 18;  // Below arrow
    display->setCursor(toX, toY);
    display->print(toPosition);

    // Update display (check displayOn state)
    if (displayOn) {
        display->display();
    } else {
        display->clearDisplay();
        display->display();
    }
}