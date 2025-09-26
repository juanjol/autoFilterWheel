#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/**
 * Display Manager for OLED screen
 * Handles all display operations and layouts
 */
class DisplayManager {
private:
    Adafruit_SSD1306* display;

    // Display configuration
    uint8_t screenWidth;
    uint8_t screenHeight;
    int8_t resetPin;
    uint8_t xOffset;        // Horizontal offset for centering (0.42" OLED)

    // Update timing
    unsigned long lastUpdate;
    uint16_t updateInterval;

    // Display state
    bool displayEnabled;
    bool needsUpdate;

    // Layout constants for 0.42" OLED (72x40 visible area)
    static constexpr uint8_t STATUS_LINE_Y = 24;
    static constexpr uint8_t POSITION_LINE_Y = 36;
    static constexpr uint8_t FILTER_NAME_LINE_Y = 52;

    // Font sizes
    static constexpr uint8_t SMALL_FONT_HEIGHT = 8;
    static constexpr uint8_t LARGE_FONT_HEIGHT = 16;

public:
    /**
     * Constructor
     * @param width Screen width in pixels
     * @param height Screen height in pixels
     * @param wire I2C interface
     * @param resetPin Reset pin (-1 if not used)
     * @param xOffset Horizontal offset for centering
     */
    DisplayManager(uint8_t width, uint8_t height, TwoWire* wire,
                   int8_t resetPin = -1, uint8_t xOffset = 30);

    /**
     * Initialize display
     * @param address I2C address (default 0x3C)
     * @return true if successful
     */
    bool init(uint8_t address = 0x3C);

    /**
     * Update display if needed (call from main loop)
     */
    void update();

    /**
     * Force immediate display update
     */
    void forceUpdate();

    /**
     * Enable/disable display updates
     */
    void setEnabled(bool enabled);
    bool isEnabled() const;

    /**
     * Set update interval in milliseconds
     */
    void setUpdateInterval(uint16_t intervalMs);

    /**
     * Display system status
     * @param status Status text ("READY", "MOVING", "ERROR", etc.)
     */
    void showStatus(const char* status);

    /**
     * Display current position
     * @param position Current filter position (1-8)
     * @param maxPosition Maximum filter position
     */
    void showPosition(uint8_t position, uint8_t maxPosition);

    /**
     * Display filter name
     * @param filterName Name of current filter
     */
    void showFilterName(const char* filterName);

    /**
     * Display complete filter wheel state
     * @param status System status
     * @param position Current position
     * @param maxPosition Maximum position
     * @param filterName Current filter name
     * @param isMoving Whether motor is moving
     */
    void showFilterWheelState(const char* status, uint8_t position,
                              uint8_t maxPosition, const char* filterName,
                              bool isMoving = false);

    /**
     * Show calibration progress
     * @param step Current calibration step
     * @param totalSteps Total calibration steps
     * @param message Calibration message
     */
    void showCalibrationProgress(uint8_t step, uint8_t totalSteps, const char* message);

    /**
     * Show error message
     * @param errorCode Error code
     * @param errorMessage Error description
     */
    void showError(uint8_t errorCode, const char* errorMessage);

    /**
     * Show configuration menu
     * @param menuItem Current menu item
     * @param value Current value
     */
    void showConfigMenu(const char* menuItem, const char* value);

    /**
     * Clear display
     */
    void clear();

    /**
     * Display splash screen
     */
    void showSplashScreen();

    /**
     * Display version information
     */
    void showVersionInfo(const char* version, const char* driver);

    /**
     * Get display dimensions
     */
    uint8_t getWidth() const { return screenWidth; }
    uint8_t getHeight() const { return screenHeight; }
    uint8_t getXOffset() const { return xOffset; }

    /**
     * Test display functionality
     */
    void runDisplayTest();

private:
    /**
     * Internal update method
     */
    void performUpdate();

    /**
     * Center text horizontally
     */
    uint8_t centerTextX(const char* text, uint8_t textSize = 1);

    /**
     * Draw centered text
     */
    void drawCenteredText(const char* text, uint8_t y, uint8_t textSize = 1);

    /**
     * Truncate text to fit display width
     */
    void truncateText(char* buffer, const char* text, uint8_t maxChars);
};