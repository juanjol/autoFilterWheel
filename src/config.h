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
#define STEPS_PER_REVOLUTION 2150  // 28BYJ-48 has 2048 steps per revolution (64 * 32)

// Motor speed and acceleration
#define MAX_MOTOR_SPEED 430.0      // Maximum steps per second
#define MOTOR_ACCELERATION 1000.0  // Steps per second squared (increased for better response)
#define MOTOR_SPEED 300.0          // Normal operating speed

// ============================================
// MOTOR DRIVER CONFIGURATION
// ============================================

// Select motor driver type (uncomment only one)
#define MOTOR_DRIVER_ULN2003    // ULN2003 with 28BYJ-48 stepper
//#define MOTOR_DRIVER_TMC2209    // TMC2209 with NEMA17 or similar bipolar stepper (also supports TMC2208)
//#define MOTOR_DRIVER_TMC2130    // TMC2130 with NEMA17 or similar bipolar stepper (SPI control)
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
  // TMC2209/TMC2208 Driver pins for bipolar stepper
  #define MOTOR_STEP_PIN 2     // Step pulse
  #define MOTOR_DIR_PIN 3      // Direction
  #define MOTOR_ENABLE_PIN 4   // Enable (active low)
  #define TMC_RX_PIN 7         // UART RX for TMC2209 configuration (GPIO7)
  #define TMC_TX_PIN 10        // UART TX for TMC2209 configuration (GPIO10)

  // TMC2209 UART Configuration
  #define TMC_SERIAL_BAUD 115200  // UART baud rate for TMC communication
  #define TMC_R_SENSE 0.11f       // Sense resistor value (0.11 ohms typical for SilentStepStick)

  // Default motor configuration (can be changed via commands)
  #define DEFAULT_MICROSTEPS 16      // Default microstepping (1, 2, 4, 8, 16, 32, 64, 128, 256)
  #define DEFAULT_MOTOR_CURRENT 800  // Default RMS current in mA (max depends on driver cooling)
  #define MAX_MOTOR_CURRENT 1500     // Maximum allowed RMS current in mA
  #define MIN_MOTOR_CURRENT 100      // Minimum RMS current in mA

  // TMC2209 StealthChop and SpreadCycle configuration
  #define USE_STEALTHCHOP true       // Enable StealthChop (quiet mode)
  #define STEALTHCHOP_THRESHOLD 100  // Velocity threshold for StealthChop
  #define USE_COOLSTEP false         // Enable CoolStep (dynamic current control)

  // Motor specifications (adjust for your motor)
  #define MOTOR_STEPS_PER_REV 200   // Full steps per revolution (200 for 1.8° motor, 400 for 0.9°)
  #define MOTOR_HOLD_MULTIPLIER 0.5f // Hold current multiplier (0.0 to 1.0)

  // TMC2209 driver address (0-3, only if using multiple drivers on same UART)
  #define TMC_DRIVER_ADDRESS 0

  // Aliases for backward compatibility
  #define MOTOR_RX_PIN TMC_RX_PIN
  #define MOTOR_TX_PIN TMC_TX_PIN
  #define MOTOR_MICROSTEPS DEFAULT_MICROSTEPS
  #define MOTOR_CURRENT_MA DEFAULT_MOTOR_CURRENT
#endif

#ifdef MOTOR_DRIVER_TMC2130
  // TMC2130 Driver pins for bipolar stepper with SPI control
  #define MOTOR_STEP_PIN 2     // Step pulse
  #define MOTOR_DIR_PIN 3      // Direction
  #define MOTOR_ENABLE_PIN 4   // Enable (active low)
  #define TMC_CS_PIN 10        // SPI Chip Select for TMC2130

  // TMC2130 SPI Configuration (uses hardware SPI)
  #define TMC_R_SENSE 0.11f    // Sense resistor value (0.11 ohms typical for SilentStepStick)

  // Default motor configuration (can be changed via commands)
  #define DEFAULT_MICROSTEPS 16      // Default microstepping (1, 2, 4, 8, 16, 32, 64, 128, 256)
  #define DEFAULT_MOTOR_CURRENT 800  // Default RMS current in mA (max depends on driver cooling)
  #define MAX_MOTOR_CURRENT 1500     // Maximum allowed RMS current in mA
  #define MIN_MOTOR_CURRENT 100      // Minimum RMS current in mA

  // TMC2130 StealthChop and SpreadCycle configuration
  #define USE_STEALTHCHOP true       // Enable StealthChop (quiet mode)
  #define STEALTHCHOP_THRESHOLD 100  // Velocity threshold for StealthChop
  #define USE_STALLGUARD true        // Enable StallGuard (stall detection)
  #define STALLGUARD_THRESHOLD 8     // StallGuard threshold (lower = more sensitive)

  // Motor specifications (adjust for your motor)
  #define MOTOR_STEPS_PER_REV 200   // Full steps per revolution (200 for 1.8° motor, 400 for 0.9°)
  #define MOTOR_HOLD_MULTIPLIER 0.5f // Hold current multiplier (0.0 to 1.0)

  // Aliases for backward compatibility
  #define MOTOR_MICROSTEPS DEFAULT_MICROSTEPS
  #define MOTOR_CURRENT_MA DEFAULT_MOTOR_CURRENT
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
#define AS5600_INVERT_DIRECTION true   // Invert encoder reading direction (360° - angle)

// Angle calibration and control
#define ANGLE_TOLERANCE 5.0        // Degrees tolerance for position detection (verification only)
#define ANGLE_CONTROL_TOLERANCE 0.8  // Degrees tolerance for encoder-based motor control (< 1°)
#define AS5600_OFFSET 0.0          // Offset angle in degrees (set during calibration)
#define ANGLE_CONTROL_MAX_ITERATIONS 30  // Maximum control loop iterations

// PID Controller Parameters for Angle Control
#define ANGLE_PID_KP 4.5f          // Proportional gain
#define ANGLE_PID_KI 0.01f         // Integral gain
#define ANGLE_PID_KD 0.3f          // Derivative gain
#define ANGLE_PID_INTEGRAL_MAX 100.0f  // Maximum integral accumulation (anti-windup)
#define ANGLE_PID_OUTPUT_MIN 10    // Minimum motor steps per iteration
#define ANGLE_PID_OUTPUT_MAX 2000   // Maximum motor steps per iteration
#define ANGLE_PID_SETTLING_TIME 150  // Delay in ms after each movement

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

// Display rotation (set to true to rotate display 180 degrees)
#define OLED_ROTATION_180 false    // Default: normal orientation
// Note: Rotation can be changed at runtime via serial command #ROTATE

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

// Motor configuration commands
#define CMD_SET_MOTOR_SPEED "MS"        // Set motor speed (MS1000)
#define CMD_SET_MAX_SPEED "MXS"         // Set maximum speed (MXS2000)
#define CMD_SET_ACCELERATION "MA"       // Set acceleration (MA1000)
#define CMD_GET_MOTOR_CONFIG "GMC"      // Get motor configuration
#define CMD_RESET_MOTOR_CONFIG "RMC"    // Reset motor config to defaults
#define CMD_SET_DISABLE_DELAY "MDD"     // Set motor disable delay (MDD2000)

// TMC2209 specific commands
#define CMD_SET_MICROSTEPS "TMC_MS"     // Set microsteps (TMC_MS16, TMC_MS32, etc.)
#define CMD_SET_CURRENT "TMC_CUR"        // Set motor current in mA (TMC_CUR800)
#define CMD_GET_TMC_STATUS "TMC_STATUS" // Get TMC2209 status and diagnostics
#define CMD_SET_STEALTHCHOP "TMC_SC"    // Enable/disable StealthChop (TMC_SC0, TMC_SC1)
#define CMD_GET_TMC_TEMP "TMC_TEMP"     // Get driver temperature status
#define CMD_TMC_RESET "TMC_RESET"        // Reset TMC2209 to defaults
#define CMD_SET_HOLD_CURRENT "TMC_HOLD"  // Set hold current multiplier (TMC_HOLD50 = 50%)

// Display commands
#define CMD_ROTATE_DISPLAY "ROTATE"     // Rotate display 180 degrees (ROTATE0, ROTATE1)
#define CMD_GET_DISPLAY_INFO "DISPLAY"  // Get display configuration info

// Encoder debugging commands
#define CMD_GET_ENCODER_STATUS "ENCSTATUS"  // Get encoder status (angle, direction, health)
#define CMD_GET_ROTATION_DIR "ENCDIR"       // Get current rotation direction

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
#define EEPROM_MOTOR_CONFIG_FLAG 0x110  // Motor config flag (0xDD when saved)
#define EEPROM_MOTOR_SPEED 0x114      // Motor speed (uint16_t)
#define EEPROM_MAX_MOTOR_SPEED 0x118  // Maximum motor speed (uint16_t)
#define EEPROM_MOTOR_ACCELERATION 0x11C // Motor acceleration (uint16_t)
#define EEPROM_MOTOR_DISABLE_DELAY 0x120 // Motor disable delay (uint16_t)
#define EEPROM_TMC_CONFIG_FLAG 0x130    // TMC2209 config flag (0xFF when saved)
#define EEPROM_TMC_MICROSTEPS 0x134     // TMC2209 microsteps (uint16_t)
#define EEPROM_TMC_CURRENT 0x138        // TMC2209 motor current in mA (uint16_t)
#define EEPROM_TMC_STEALTHCHOP 0x13C    // TMC2209 StealthChop enabled (uint8_t)
#define EEPROM_TMC_HOLD_MULT 0x140      // TMC2209 hold current multiplier (float)
#define EEPROM_DISPLAY_CONFIG_FLAG 0x144  // Display config flag (0xAA when saved)
#define EEPROM_DISPLAY_ROTATION 0x148     // Display rotation (uint8_t: 0=normal, 1=180°)
#define MAX_FILTER_NAME_LENGTH 15    // Maximum characters per filter name (+ 1 for null terminator)
#define MIN_FILTER_COUNT 3           // Minimum number of filters
#define MAX_FILTER_COUNT 9           // Maximum number of filters (3-9 supported)

// Safety and timeouts
#define MOVEMENT_TIMEOUT 10000      // Maximum time for movement in ms
#define POSITION_RETRY_COUNT 3      // Number of retries for positioning

// Manual stepping configuration
#define MAX_MANUAL_STEPS 4096       // Maximum steps allowed in one manual command (2 revolutions)
#define MIN_MANUAL_STEPS 1          // Minimum steps for manual movement

// Motor power management
#define MOTOR_DISABLE_DELAY 1000    // Time in ms to keep motor powered after movement (reduced to 1 second)
#define AUTO_DISABLE_MOTOR true     // Automatically disable motor after movement
#define MOTOR_HOLD_CURRENT false    // Set to true if you want to keep position with reduced current

// Debug mode (set to 1 to enable debug output)
#define DEBUG_MODE 0

// ============================================
// ERROR CODES
// ============================================

#define FW_ERROR_NONE 0
#define FW_ERROR_MOTOR_TIMEOUT 1
#define FW_ERROR_ENCODER_FAULT 2
#define FW_ERROR_INVALID_POSITION 3
#define FW_ERROR_CALIBRATION_FAILED 4
#define FW_ERROR_EEPROM_FAULT 5
#define FW_ERROR_COMMUNICATION 6

// ============================================
// FIRMWARE INFORMATION
// ============================================

#define FIRMWARE_VERSION "2.0.0"
#define DEVICE_NAME "ESP32-C3 Filter Wheel"
#define MANUFACTURER "DIY Astronomy"
#define DEVICE_ID "ESP32FW-PID-V2.0"     // Unique device identifier for ASCOM

#endif // CONFIG_H