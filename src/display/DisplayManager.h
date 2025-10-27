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
    bool displayOn;         // Physical display on/off state
    bool needsUpdate;
    bool rotation180;       // True if display is rotated 180 degrees
    uint8_t displayMode;    // 0=minimal (large number), 1=detailed (full info)
    uint8_t brightness;     // Display brightness/contrast (0-255)
    uint8_t powerMode;      // Power mode: 0=auto, 1=always on, 2=always off
    uint16_t autoOffTimeout; // Auto-off timeout in seconds (0=never)
    unsigned long lastActivityTime; // Last time display had activity
    unsigned long displayOffTime;   // Time when display should turn off
    int16_t scrollOffset;           // Current horizontal scroll offset for text
    unsigned long lastScrollTime;   // Last time scroll was updated
    uint16_t scrollDelay;           // Delay between scroll steps (ms)

    // Layout constants for 0.42" OLED (72x40 visible area)
    // Normal rotation (content at bottom of 128x64 buffer)
    static constexpr uint8_t STATUS_LINE_Y_NORMAL = 24;
    static constexpr uint8_t POSITION_LINE_Y_NORMAL = 36;
    static constexpr uint8_t FILTER_NAME_LINE_Y_NORMAL = 52;

    // Rotated 180° (content at top of buffer, but displayed at bottom)
    static constexpr uint8_t STATUS_LINE_Y_ROTATED = 0;
    static constexpr uint8_t POSITION_LINE_Y_ROTATED = 12;
    static constexpr uint8_t FILTER_NAME_LINE_Y_ROTATED = 28;

    // Font sizes
    static constexpr uint8_t SMALL_FONT_HEIGHT = 8;
    static constexpr uint8_t LARGE_FONT_HEIGHT = 16;

    // Helper methods for rotation-aware coordinates
    inline uint8_t getStatusLineY() const {
        return rotation180 ? STATUS_LINE_Y_ROTATED : STATUS_LINE_Y_NORMAL;
    }
    inline uint8_t getPositionLineY() const {
        return rotation180 ? POSITION_LINE_Y_ROTATED : POSITION_LINE_Y_NORMAL;
    }
    inline uint8_t getFilterNameLineY() const {
        return rotation180 ? FILTER_NAME_LINE_Y_ROTATED : FILTER_NAME_LINE_Y_NORMAL;
    }

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

    /**
     * Set display rotation
     * @param rotate180 True to rotate display 180 degrees
     */
    void setRotation(bool rotate180);

    /**
     * Get current rotation state
     * @return True if display is rotated 180 degrees
     */
    bool isRotated180() const { return rotation180; }

    /**
     * Set display mode
     * @param mode 0=minimal (large number), 1=detailed (full info)
     */
    void setDisplayMode(uint8_t mode);

    /**
     * Get current display mode
     * @return Current display mode
     */
    uint8_t getDisplayMode() const { return displayMode; }

    /**
     * Set display brightness/contrast
     * @param brightness Brightness level (0-255, default 255)
     */
    void setBrightness(uint8_t brightness);

    /**
     * Get current brightness level
     * @return Current brightness (0-255)
     */
    uint8_t getBrightness() const { return brightness; }

    /**
     * Turn display physically on/off
     */
    void turnOn();
    void turnOff();
    bool isDisplayOn() const { return displayOn; }

    /**
     * Set display power mode
     * @param mode 0=auto, 1=always on, 2=always off
     */
    void setPowerMode(uint8_t mode);
    uint8_t getPowerMode() const { return powerMode; }

    /**
     * Set auto-off timeout
     * @param seconds Seconds before auto-off (0=never)
     */
    void setAutoOffTimeout(uint16_t seconds);
    uint16_t getAutoOffTimeout() const { return autoOffTimeout; }

    /**
     * Reset activity timer (call when user interacts)
     */
    void resetActivityTimer();

    /**
     * Show transition screen during movement (e.g., "1 → 3")
     */
    void showTransition(uint8_t fromPosition, uint8_t toPosition);

    /**
     * Save display configuration to EEPROM
     */
    void saveDisplayConfig();

    /**
     * Load display configuration from EEPROM
     */
    void loadDisplayConfig();
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
     * Show minimal display mode (large number + filter name)
     */
    void showFilterWheelStateMinimal(uint8_t position, uint8_t totalFilters, const char* filterName, bool moving);

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