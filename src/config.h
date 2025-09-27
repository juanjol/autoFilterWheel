#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// FILTER WHEEL CONFIGURATION
// ============================================

// Number of filter positions (modifiable)
#define NUM_FILTERS 5

// Filter names - Customize these for your filters
// Maximum 10 characters per name for good display on small OLED
#define FILTER_NAME_1 "Luminance"     // Clear/Luminance filter
#define FILTER_NAME_2 "Red"           // Red filter
#define FILTER_NAME_3 "Green"         // Green filter
#define FILTER_NAME_4 "Blue"          // Blue filter
#define FILTER_NAME_5 "H-Alpha"       // H-alpha narrowband

// Alternative astronomy filter examples:
// "L", "R", "G", "B", "Ha"        - LRGB + H-alpha
// "Clear", "Ha", "OIII", "SII", "Dark"  - Narrowband set
// "UV", "B", "V", "R", "I"         - Photometric filters
// "ND1", "ND2", "ND3", "ND4", "Moon"  - Neutral density + Moon

// Motor configuration for 28BYJ-48
#define STEPS_PER_REVOLUTION 2048  // 28BYJ-48 has 2048 steps per revolution (64 * 32)
#define STEPS_PER_FILTER (STEPS_PER_REVOLUTION / NUM_FILTERS)  // Steps between each filter position

// Motor speed and acceleration
#define MAX_MOTOR_SPEED 500.0      // Maximum steps per second
#define MOTOR_ACCELERATION 200.0   // Steps per second squared
#define MOTOR_SPEED 300.0          // Normal operating speed

// Motor direction configuration
#define MOTOR_DIRECTION_MODE 0      // 0 = Unidirectional (always forward), 1 = Bidirectional (shortest path)
#define MOTOR_REVERSE_DIRECTION true   // If true, reverses the motor direction (for unidirectional mode)
// Note: In unidirectional mode, the motor always goes in one direction (1->2->3->4->5->1)
// Set MOTOR_REVERSE_DIRECTION to true if your motor turns the wrong way

// ============================================
// MOTOR DRIVER CONFIGURATION
// ============================================

// Select motor driver type (uncomment only one)
#define MOTOR_DRIVER_ULN2003    // ULN2003 with 28BYJ-48 stepper
//#define MOTOR_DRIVER_TMC2209    // TMC2209 with NEMA17 or similar bipolar stepper
//#define MOTOR_DRIVER_A4988      // A4988 with bipolar stepper
//#define MOTOR_DRIVER_DRV8825    // DRV8825 with bipolar stepper

// ============================================
// PIN DEFINITIONS - ESP32-C3
// ============================================

#ifdef MOTOR_DRIVER_ULN2003
  // ULN2003 Driver pins for 28BYJ-48 motor
  #define MOTOR_PIN1 2   // IN1 on ULN2003 -> LED A
  #define MOTOR_PIN2 3   // IN2 on ULN2003 -> LED B
  #define MOTOR_PIN3 4   // IN3 on ULN2003 -> LED C
  #define MOTOR_PIN4 10  // IN4 on ULN2003 -> LED D
#endif

#ifdef MOTOR_DRIVER_TMC2209
  // TMC2209 Driver pins for bipolar stepper
  #define MOTOR_STEP_PIN 2     // Step pulse
  #define MOTOR_DIR_PIN 3      // Direction
  #define MOTOR_ENABLE_PIN 4   // Enable (active low)
  #define MOTOR_RX_PIN 16      // UART RX for TMC2209 configuration
  #define MOTOR_TX_PIN 17      // UART TX for TMC2209 configuration
  #define MOTOR_MICROSTEPS 16  // Microstepping configuration
  #define MOTOR_CURRENT_MA 800 // Motor current in mA
#endif

#ifdef MOTOR_DRIVER_A4988
  // A4988 Driver pins for bipolar stepper
  #define MOTOR_STEP_PIN 2     // Step pulse
  #define MOTOR_DIR_PIN 3      // Direction
  #define MOTOR_ENABLE_PIN 4   // Enable (active low)
  #define MOTOR_MS1_PIN 5      // Microstep select 1
  #define MOTOR_MS2_PIN 6      // Microstep select 2
  #define MOTOR_MS3_PIN 7      // Microstep select 3
#endif

#ifdef MOTOR_DRIVER_DRV8825
  // DRV8825 Driver pins for bipolar stepper
  #define MOTOR_STEP_PIN 2     // Step pulse
  #define MOTOR_DIR_PIN 3      // Direction
  #define MOTOR_ENABLE_PIN 4   // Enable (active low)
  #define MOTOR_M0_PIN 5       // Microstep mode 0
  #define MOTOR_M1_PIN 6       // Microstep mode 1
  #define MOTOR_M2_PIN 7       // Microstep mode 2
#endif

// I2C pins for AS5600 and OLED (ESP32-C3 with 0.42" OLED)
#define I2C_SDA 5      // SDA pin (GPIO5 for 0.42" OLED)
#define I2C_SCL 6      // SCL pin (GPIO6 for 0.42" OLED)

// Status LED
#define LED_PIN 8     // Built-in or external LED for status

// Optional manual control buttons (if needed)
#define BUTTON_NEXT 9  // Button to move to next filter (changed from 6)
#define BUTTON_PREV 7  // Button to move to previous filter

// ============================================
// AS5600 MAGNETIC ENCODER CONFIGURATION
// ============================================

#define AS5600_ADDRESS 0x36        // I2C address of AS5600
#define AS5600_RAW_ANGLE_REGISTER 0x0C  // Register for raw angle

// Angle calibration
#define ANGLE_TOLERANCE 5.0        // Degrees tolerance for position detection
#define AS5600_OFFSET 0.0          // Offset angle in degrees (set during calibration)

// Position angles for each filter (degrees)
// These will be automatically calculated based on NUM_FILTERS
// but can be manually adjusted if needed
#define FILTER_ANGLE_1 0.0
#define FILTER_ANGLE_2 72.0   // 360/5 = 72 degrees
#define FILTER_ANGLE_3 144.0
#define FILTER_ANGLE_4 216.0
#define FILTER_ANGLE_5 288.0

// ============================================
// OLED DISPLAY CONFIGURATION
// ============================================

#define SCREEN_WIDTH 128    // OLED display buffer width in pixels
#define SCREEN_HEIGHT 64    // OLED display buffer height in pixels
// Actual visible area for 0.42" OLED
#define OLED_WIDTH 72       // Actual visible width
#define OLED_HEIGHT 40      // Actual visible height
#define OLED_X_OFFSET 30    // X offset for centering
#define OLED_Y_OFFSET 0     // Y offset - start from top (0.42" OLED shows top portion)
#define OLED_RESET -1       // Reset pin (not used for I2C)
#define OLED_ADDRESS 0x3C   // I2C address for OLED (0x3C or 0x3D)

// Display update interval
#define DISPLAY_UPDATE_INTERVAL 100  // milliseconds

// ============================================
// SERIAL COMMUNICATION
// ============================================

#define SERIAL_BAUD_RATE 115200     // Baud rate for serial communication
#define COMMAND_TIMEOUT 1000        // Command timeout in milliseconds

// ============================================
// ASCOM PROTOCOL COMMANDS
// ============================================

// Command structure: #CMD[params]\r
// Response structure: [response]\n

#define CMD_GET_POSITION "GP"       // Get current position
#define CMD_MOVE_POSITION "MP"      // Move to position (MP1, MP2, etc.)
#define CMD_SET_POSITION "SP"       // Set current position (SP1, SP2, etc.)
#define CMD_CALIBRATE "CAL"         // Calibrate home position
#define CMD_STATUS "STATUS"         // Get system status
#define CMD_GET_FILTERS "GF"        // Get number of filters
#define CMD_SET_FILTER_COUNT "FC"   // Set filter count (FC3, FC4, FC5, etc.)
#define CMD_GET_FILTER_NAME "GN"    // Get filter name (GN1, GN2, etc.)
#define CMD_SET_FILTER_NAME "SN"    // Set filter name (SN1:Luminance, SN2:Red, etc.)
#define CMD_VERSION "VER"           // Get firmware version
#define CMD_DEVICE_ID "ID"          // Get device identification
#define CMD_STOP "STOP"            // Emergency stop
#define CMD_STEP_FORWARD "SF"       // Step forward X steps (SF100)
#define CMD_STEP_BACKWARD "SB"      // Step backward X steps (SB50)
#define CMD_STEP_TO "ST"           // Step to absolute position (ST1024)
#define CMD_MOTOR_ENABLE "ME"       // Enable motor power
#define CMD_MOTOR_DISABLE "MD"      // Disable motor power

// Revolution calibration commands
#define CMD_START_REV_CAL "REVCAL"  // Start full revolution calibration
#define CMD_REV_CAL_ADJUST_PLUS "RCP"  // Add steps during revolution calibration (RCP10)
#define CMD_REV_CAL_ADJUST_MINUS "RCM" // Subtract steps during revolution calibration (RCM5)
#define CMD_FINISH_REV_CAL "RCFIN"  // Finish revolution calibration and save result

// Backlash calibration commands
#define CMD_START_BACKLASH_CAL "BLCAL"  // Start backlash calibration
#define CMD_BACKLASH_STEP "BLS"         // Manual step during backlash calibration (BLS10)
#define CMD_BACKLASH_MARK "BLM"         // Mark movement detected during backlash calibration
#define CMD_FINISH_BACKLASH_CAL "BLFIN" // Finish backlash calibration and save result

// Motor configuration commands
#define CMD_SET_MOTOR_SPEED "MS"        // Set motor speed (MS1000)
#define CMD_SET_MAX_SPEED "MXS"         // Set maximum speed (MXS2000)
#define CMD_SET_ACCELERATION "MA"       // Set acceleration (MA1000)
#define CMD_GET_MOTOR_CONFIG "GMC"      // Get motor configuration
#define CMD_RESET_MOTOR_CONFIG "RMC"    // Reset motor config to defaults
#define CMD_SET_DISABLE_DELAY "MDD"     // Set motor disable delay (MDD2000)

// Motor direction commands
#define CMD_SET_DIRECTION_MODE "MDM"    // Set direction mode (MDM0=unidirectional, MDM1=bidirectional)
#define CMD_SET_REVERSE_MODE "MRV"      // Set reverse mode (MRV0=normal, MRV1=reversed)
#define CMD_GET_DIRECTION_CONFIG "GDC"  // Get direction configuration

// ============================================
// SYSTEM CONFIGURATION
// ============================================

// EEPROM addresses for storing calibration
#define EEPROM_SIZE 512
#define EEPROM_CALIB_FLAG 0x00     // Address for calibration flag
#define EEPROM_OFFSET_ADDR 0x04    // Address for AS5600 offset
#define EEPROM_CURRENT_POS 0x08    // Address for current position
#define EEPROM_FILTER_NAMES_FLAG 0x0C // Filter names stored flag
#define EEPROM_POSITIONS_ADDR 0x10 // Starting address for filter positions
#define EEPROM_FILTER_COUNT 0x0D     // Address for dynamic filter count
#define EEPROM_FILTER_NAMES_ADDR 0x20 // Starting address for filter names (16 chars each)
#define EEPROM_REV_CAL_FLAG 0x100     // Revolution calibration flag (0xBB when calibrated)
#define EEPROM_STEPS_PER_REV 0x104    // Calibrated steps per revolution (uint16_t)
#define EEPROM_BACKLASH_FLAG 0x108    // Backlash calibration flag (0xCC when calibrated)
#define EEPROM_BACKLASH_STEPS 0x10C   // Calibrated backlash compensation steps (uint8_t)
#define EEPROM_MOTOR_CONFIG_FLAG 0x110  // Motor config flag (0xDD when saved)
#define EEPROM_MOTOR_SPEED 0x114      // Motor speed (uint16_t)
#define EEPROM_MAX_MOTOR_SPEED 0x118  // Maximum motor speed (uint16_t)
#define EEPROM_MOTOR_ACCELERATION 0x11C // Motor acceleration (uint16_t)
#define EEPROM_MOTOR_DISABLE_DELAY 0x120 // Motor disable delay (uint16_t)
#define EEPROM_DIRECTION_FLAG 0x124     // Direction config flag (0xEE when saved)
#define EEPROM_DIRECTION_MODE 0x128     // Direction mode (uint8_t: 0=unidirectional, 1=bidirectional)
#define EEPROM_REVERSE_MODE 0x12C       // Reverse mode (uint8_t: 0=normal, 1=reversed)
#define MAX_FILTER_NAME_LENGTH 15    // Maximum characters per filter name (+ 1 for null terminator)
#define MIN_FILTER_COUNT 3           // Minimum number of filters
#define MAX_FILTER_COUNT 8           // Maximum number of filters (hardware/EEPROM limit)

// Safety and timeouts
#define MOVEMENT_TIMEOUT 10000      // Maximum time for movement in ms
#define POSITION_RETRY_COUNT 3      // Number of retries for positioning
#define BACKLASH_COMPENSATION 10    // Steps for backlash compensation

// Manual stepping configuration
#define MAX_MANUAL_STEPS 2048       // Maximum steps allowed in one manual command
#define MIN_MANUAL_STEPS 1          // Minimum steps for manual movement

// Motor power management
#define MOTOR_DISABLE_DELAY 1000    // Time in ms to keep motor powered after movement (reduced to 1 second)
#define MOTOR_ENABLE_PIN -1         // Pin to control motor power (set to valid GPIO if using external enable)
#define AUTO_DISABLE_MOTOR true     // Automatically disable motor after movement
#define MOTOR_HOLD_CURRENT false    // Set to true if you want to keep position with reduced current

// Debug mode (set to 1 to enable debug output)
#define DEBUG_MODE 0

// ============================================
// ERROR CODES
// ============================================

#define ERROR_NONE 0
#define ERROR_MOTOR_TIMEOUT 1
#define ERROR_ENCODER_FAULT 2
#define ERROR_INVALID_POSITION 3
#define ERROR_CALIBRATION_FAILED 4
#define ERROR_EEPROM_FAULT 5
#define ERROR_COMMUNICATION 6

// ============================================
// FIRMWARE INFORMATION
// ============================================

#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_NAME "ESP32-C3 Filter Wheel"
#define MANUFACTURER "DIY Astronomy"
#define DEVICE_ID "ESP32FW-5POS-V1.0"     // Unique device identifier for ASCOM

#endif // CONFIG_H