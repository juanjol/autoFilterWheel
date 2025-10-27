# ESP32-C3 Filter Wheel Controller - Command Reference

Complete reference for all serial commands supported by the ESP32-C3 Filter Wheel Controller firmware.

## Command Format

Commands are sent via serial communication at 115200 baud rate:
- Commands can be prefixed with `#` (optional)
- Commands are case-insensitive
- Commands end with newline (`\n`) or carriage return (`\r`)
- Responses are immediate

## Basic Movement Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `GP` | Get current position | None | `#GP` | `P3` | Returns current filter position (1-9) |
| `MP[X]` | Move to position | X = Position (1-9) | `#MP2` | `M2` | Moves to specified filter position using PID control |
| `SP[X]` | Set current position | X = Position (1-9) | `#SP1` | `S1` | Sets current position without moving |

## Filter Configuration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `GF` | Get filter count | None | `#GF` | `F5` | Returns number of filters (3-9) |
| `FC[X]` | Set filter count | X = Count (3-9) | `#FC6` | `FC6` | Sets total number of filters |
| `GN` | Get all filter names | None | `#GN` | `NAMES:Luminance,Red,Green,Blue,H-Alpha` | Comma-separated list |
| `GN[X]` | Get specific filter name | X = Position (1-9) | `#GN2` | `N2:Red` | Returns name for position X |
| `SN[X]:Name` | Set filter name | X = Position, Name = Filter name | `#SN1:Luminance` | `SN1:Luminance` | Max 15 characters, stored in EEPROM |

## System Information Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `STATUS` | Get system status | None | `#STATUS` | `STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=180.5,ERROR=0.2` | Complete system state with encoder angle |
| `GETCONFIG` | Get complete configuration | None | `#GETCONFIG` | Multi-line response (see below) | Returns all system configuration in one command |
| `ID` | Get device identifier | None | `#ID` | `DEVICE_ID:ESP32FW-PID-V2.0` | Device identification |
| `VER` | Get firmware version | None | `#VER` | `VERSION:2.0.1` | Current firmware version |
| `CAL` | Calibrate encoder offset | None | `#CAL` | `CALIBRATED` | Sets current angle as position 1 (0°) |
| `STOP` | Emergency stop | None | `#STOP` | `STOPPED` | Immediately stops all movement |

### GETCONFIG Response Format

The `GETCONFIG` command returns all system configuration and status in a single multi-line response:

```
FILTER_COUNT:5
FILTER_NAME:1:Luminance
FILTER_NAME:2:Red
FILTER_NAME:3:Green
FILTER_NAME:4:Blue
FILTER_NAME:5:H-Alpha
MOTOR_SPEED:4000
MOTOR_MAX_SPEED:5000
MOTOR_ACCEL:100000
MOTOR_DISABLE_DELAY:1000
MOTOR_STEPS_PER_REV:35500
MOTOR_INV:0
ENC_INV:0
DISPLAY_SIZE:128x64
DISPLAY_ROTATION:0
DISPLAY_ENABLED:1
DISPLAY_BRIGHTNESS:128
DISPLAY_MODE:0
DISPLAY_POWER_MODE:0
DISPLAY_TIMEOUT:30
STATUS_POSITION:1
STATUS_MOVING:0
STATUS_CALIBRATED:1
STATUS_ANGLE:0.00
CONFIG_END
```

**Benefits:**
- Single command replaces ~15 individual GET commands
- Reduces communication overhead from ~6-10 seconds to ~1-2 seconds
- Ideal for driver initialization and periodic status updates
- All values guaranteed to be from the same point in time (atomic snapshot)

## Manual Stepping Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `SF[X]` | Step forward | X = Steps (1-1000) | `#SF100` | `SF100` | Manual forward stepping (for calibration/testing) |
| `SB[X]` | Step backward | X = Steps (1-1000) | `#SB50` | `SB50` | Manual backward stepping (for calibration/testing) |

## Motor Configuration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `MS[X]` | Set motor speed | X = Speed (50-3000) | `#MS1000` | `MS1000` | Steps per second |
| `MXS[X]` | Set maximum speed | X = Max speed (100-5000) | `#MXS2000` | `MXS2000` | Maximum steps per second |
| `MA[X]` | Set motor acceleration | X = Acceleration (50-2000) | `#MA800` | `MA800` | Steps per second squared |
| `MDD[X]` | Set motor disable delay | X = Delay (500-10000) | `#MDD2000` | `MDD2000` | Auto-disable delay in milliseconds |
| `GMC` | Get motor configuration | None | `#GMC` | `MOTOR_CONFIG:SPEED=1000,MAX_SPEED=2000,ACCELERATION=800,DISABLE_DELAY=1000` | All motor parameters |
| `RMC` | Reset motor config | None | `#RMC` | `MOTOR_CONFIG_RESET` | Restore default motor settings |

## Custom Angle Calibration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `SETANG[X]:[angle]` | Set custom angle for position | X = Position (1-9), angle = 0.0-359.99 | `#SETANG1:0.0` | `ANG1_SET:0.0` | Store custom angle for position X |
| `GETANG` | Get all custom angles | None | `#GETANG` | `ANGLES:0.0,68.5,142.3,215.8,289.0` | Returns all configured angles |
| `GETANG[X]` | Get angle for position | X = Position (1-9) | `#GETANG2` | `ANG2:68.5` | Returns angle for specific position |
| `CLEARANG` | Clear custom angles | None | `#CLEARANG` | `ANGLES_CLEARED` | Revert to uniform angle distribution |

## Motor Power Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `ME` | Enable motor power | None | `#ME` | `MOTOR_ENABLED` | Manually enable motor |
| `MD` | Disable motor power | None | `#MD` | `MOTOR_DISABLED` | Manually disable motor |

## Encoder Diagnostic Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `ANGLE` | Get encoder angle | None | `#ANGLE` | `ANGLE:180.5` | Current absolute encoder angle (0-359.99°) |
| `ENCSTATUS` | Get encoder status | None | `#ENCSTATUS` | `ENC_STATUS:OK,ANGLE=180.5,MAG=GOOD` | Encoder health and diagnostics |
| `ENCDIR` | Get rotation direction | None | `#ENCDIR` | `ENC_DIR:CW` or `ENC_DIR:CCW` | Current rotation direction |
| `ENCRAW` | Get raw encoder data | None | `#ENCRAW` | `ENC_RAW:2048,STATUS=0x20` | Raw encoder count and status register |

## Display Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `DISPLAY` | Get display info | None | `#DISPLAY` | `DISPLAY:Size=128x64,Rotation=Normal,Brightness=255` | Display configuration and status |
| `ROTATE[X]` | Rotate display 180° | X = 0 (normal) or 1 (180°), or none to toggle | `#ROTATE1` | `ROTATE1` | Rotates display orientation, persists to EEPROM |
| `DISPMODE[X]` | Set display mode | X = 0 (minimal) or 1 (detailed) | `#DISPMODE0` | `DISPMODE0:Minimal` | Minimal shows large number, detailed shows full info |
| `BRIGHT[X]` | Set display brightness | X = 0-255 | `#BRIGHT128` | `BRIGHT:128` | 0=off, 255=max brightness, persists to EEPROM |
| `DISPON` | Turn display on | None | `#DISPON` | `DISPON:OK` | Manually turn display on |
| `DISPOFF` | Turn display off | None | `#DISPOFF` | `DISPOFF:OK` | Manually turn display off |
| `DISPPOWER[X]` | Set display power mode | X = 0 (auto), 1 (always on), 2 (always off) | `#DISPPOWER0` | `DISPPOWER:0:Auto` | Auto mode uses timeout setting |
| `DISPTIMEOUT[X]` | Set auto-off timeout | X = Seconds (0-65535), 0=never | `#DISPTIMEOUT60` | `DISPTIMEOUT:60:Seconds` | Only applies when power mode is Auto |

## Command Workflows

### Initial Setup (First Time)
1. `#FC5` - Set filter count (3-9 filters)
2. `#SN1:Luminance` - Set filter names
3. `#SN2:Red`, `#SN3:Green`, etc.
4. `#GN` - Verify all filter names
5. Position wheel at filter #1 manually
6. `#CAL` - Calibrate encoder offset (sets current position as 0°)

**Auto-Homing Behavior:**
- By default (`AUTO_HOMING_GO_TO_POSITION_1 = false`), the system will detect current position on startup and only move if needed
- If you want the wheel to always return to position 1 on power-up, set `AUTO_HOMING_GO_TO_POSITION_1 = true` in config.h

### Basic Movement
1. `#MP3` - Move to filter position 3 using PID control
2. `#GP` - Verify current position
3. `#STATUS` - Check angle, error, and system state

### Custom Angle Calibration (Optional, for improved accuracy)

**Method 1: Manual Angle Entry (when angles are known)**
1. `#SETANG1:0.0` - Set position 1 to 0°
2. `#SETANG2:68.5` - Set position 2 to 68.5°
3. `#SETANG3:142.3` - Continue for all positions
4. `#GETANG` - Verify all angles

**Method 2: Interactive Positioning (measure actual positions)**
1. `#SF100` / `#SB50` - Manually adjust wheel position
2. `#ENCSTATUS` - Check current encoder angle
3. When positioned correctly: `#SETANG1:0.0` (use current angle)
4. Repeat for each position
5. `#GETANG` - Verify all calibrated angles

### Motor Tuning (if needed)
1. `#MS800` - Set motor speed (steps/second)
2. `#MA600` - Set acceleration
3. `#MXS2000` - Set maximum speed
4. `#MDD1000` - Set motor disable delay
5. `#GMC` - Verify motor configuration

### Encoder Diagnostics
1. `#ENCSTATUS` - Check encoder health and magnetic field
2. `#ANGLE` - Get current absolute angle
3. `#ENCRAW` - Get raw encoder data for troubleshooting

### Display Power Management

There are two ways to control display power:

**Method 1: Power Mode (DISPPOWER)**
- `#DISPPOWER0` - Auto mode: Display auto-off after timeout (default)
- `#DISPPOWER1` - Always on: Display never turns off automatically
- `#DISPPOWER2` - Always off: Display always off (manual control only)

**Method 2: Timeout Setting (DISPTIMEOUT)** - Only applies in Auto mode
- `#DISPTIMEOUT30` - Auto-off after 30 seconds (default)
- `#DISPTIMEOUT60` - Auto-off after 1 minute
- `#DISPTIMEOUT300` - Auto-off after 5 minutes
- `#DISPTIMEOUT0` - Never auto-off (similar to AlwaysOn, but still in Auto mode)

**Important Notes:**
- `DISPTIMEOUT` only affects behavior when `DISPPOWER=0` (Auto mode)
- To keep display always on: Use `#DISPPOWER1` OR use `#DISPPOWER0` + `#DISPTIMEOUT0`
- The difference: `DISPPOWER1` ignores all timeout settings; `DISPTIMEOUT0` keeps display on but can be changed later without switching modes

**Typical Usage:**

*Auto with 1-minute timeout:*
```
#DISPPOWER0      // Set to Auto mode
#DISPTIMEOUT60   // Auto-off after 60 seconds
```

*Always on (two equivalent ways):*
```
#DISPPOWER1      // Method 1: Power mode always-on
  OR
#DISPPOWER0 + #DISPTIMEOUT0  // Method 2: Auto mode with no timeout
```

*Always off:*
```
#DISPPOWER2      // Display always off
#DISPON          // Manually turn on temporarily
```

**Brightness Control:**
1. `#BRIGHT255` - Maximum brightness
2. `#BRIGHT128` - Medium brightness (50%)
3. `#BRIGHT64` - Low brightness (25%)
4. `#BRIGHT0` - Minimum (effectively off)

## Error Responses

| Error | Description | Cause |
|-------|-------------|--------|
| `ERROR:UNKNOWN_COMMAND` | Command not recognized | Invalid command syntax |
| `ERROR:INVALID_POSITION` | Position out of range | Position not between 1 and filter count |
| `ERROR:INVALID_COUNT` | Filter count invalid | Count not between 3-9 |
| `ERROR:INVALID_FORMAT` | Command format wrong | Missing parameters or wrong syntax |
| `ERROR:INVALID_SPEED` | Speed out of range | Motor speed not in valid range |
| `ERROR:INVALID_MAX_SPEED` | Max speed out of range | Maximum speed not in valid range |
| `ERROR:INVALID_ACCELERATION` | Acceleration out of range | Acceleration not in valid range |
| `ERROR:INVALID_DELAY` | Delay out of range | Disable delay not in valid range |
| `ERROR:NAME_TOO_LONG` | Filter name too long | Name exceeds 15 characters |
| `ERROR:INVALID_ANGLE` | Angle out of range | Angle not between 0.0-359.99° |
| `ERROR:ENCODER_NOT_AVAILABLE` | Encoder not detected | AS5600 encoder required for operation |
| `ERROR:CALIBRATION_REQUIRED` | System not calibrated | Must run #CAL command first |
| `ERROR:POSITION_TIMEOUT` | Movement timeout | Failed to reach target position |
| `ERROR:MOVEMENT_IN_PROGRESS` | Already moving | Wait for current movement to complete |

## Parameter Ranges

| Parameter | Minimum | Maximum | Default | Unit |
|-----------|---------|---------|---------|------|
| Filter Count | 3 | 9 | 5 | filters |
| Filter Position | 1 | 9 | - | position |
| Custom Angle | 0.0 | 359.99 | - | degrees |
| Motor Speed | 50 | 5000 | 4000 | steps/sec |
| Max Motor Speed | 100 | 5000 | 5000 | steps/sec |
| Motor Acceleration | 50 | 100000 | 100000 | steps/sec² |
| Motor Disable Delay | 500 | 10000 | 500 | milliseconds |
| Manual Steps | 1 | 4096 | - | steps |
| Filter Name Length | 1 | 15 | - | characters |
| Display Brightness | 0 | 255 | 255 | level |
| Display Power Mode | 0 | 2 | 0 | mode |
| Display Auto-Off Timeout | 0 | 65535 | 30 | seconds |

## EEPROM Storage

All configuration parameters are automatically saved to EEPROM and persist across power cycles:

- **Encoder offset calibration** (0x04): AS5600 angle offset (float)
- **Current position** (0x08): Last known filter position (uint8_t)
- **Filter count** (0x10): Number of configured filters (uint8_t)
- **Custom angles** (0x11-0x36): Custom angle array for positions 1-9 (9 floats)
- **Filter names** (0x40+): Custom filter names (16 bytes each, up to 15 chars + null)
- **Motor configuration** (0x114+): Speed, max speed, acceleration, disable delay
- **Display settings** (0x144+): Rotation, mode, brightness, power mode, auto-off timeout

## Debug Mode

When `DEBUG_MODE` is enabled in firmware, additional diagnostic information is sent to serial output:

- **PID Control Loop**: Iteration-by-iteration angle error, P/I/D terms, and motor steps
- **Movement Details**: Current angle, target angle, direction selection
- **Encoder Diagnostics**: Magnetic field strength, status register, raw counts
- **Configuration Changes**: EEPROM writes, parameter updates
- **Error Diagnostics**: Detailed error conditions and recovery attempts

## PID Control System

This firmware uses a closed-loop PID controller for precision positioning:

- **Target Accuracy**: < 0.8° (configurable via `ANGLE_CONTROL_TOLERANCE`)
- **Control Loop**: Maximum 30 iterations with 20ms settling time per iteration (fast response)
- **PID Parameters**: Kp=76.0, Ki=0.17, Kd=5.0 (scaled for STEPS_PER_REVOLUTION=34600)
- **Output Range**: 100-15000 steps per iteration for optimal response
- **Anti-Windup**: Integral limit of 1700.0 prevents accumulation
- **Bidirectional**: Automatically chooses shortest path and can reverse if needed
- **Non-Blocking**: Serial commands can be processed during movement
- **Real-Time Display**: Shows current angle during movement every 2 iterations

## ASCOM Integration

This command set is designed for ASCOM driver integration:

- **Position-Based Commands**: 1-indexed filter positions
- **Immediate Response Format**: All commands respond immediately
- **Status Reporting**: Real-time position, angle, and error information
- **Error Handling**: Comprehensive error messages with descriptive text
- **Device Identification**: Unique device ID and version reporting
- **Calibration Support**: Full calibration workflow accessible via serial

## Hardware Requirements

- **Microcontroller**: ESP32-C3 with integrated OLED display
- **Motor**: 28BYJ-48 stepper motor (2048 steps/revolution) with ULN2003 driver
- **Display**: 0.42" OLED (SSD1306, 128x64 with 72x40 visible area)
- **Encoder**: AS5600 magnetic rotary encoder (**required** for PID-based positioning)
  - 12-bit resolution (4096 counts/revolution = 0.088°/count)
  - 3.3V power supply
  - I2C interface (address 0x36)
  - Diametrically magnetized 6mm neodymium magnet
- **Power**: USB 5V for ESP32-C3, external 5V/1A minimum for motor
- **Communication**: USB serial at 115200 baud, LF line ending

## Firmware Version

This command reference is for firmware version **2.0.1** (PID-based encoder control).

**New in v2.0.1:**
- **Display Power Management**: Auto-off with configurable timeout, always-on, or always-off modes
- **Display Brightness Control**: 0-255 levels with EEPROM persistence
- **Transition Screen**: Shows "FROM → TO" during filter changes
- **Non-Blocking Movements**: Serial commands responsive during movement
- **Instant Acceleration**: 100,000 steps/s² for rapid response
- **Optimized PID**: Corrected step calculations for accurate first movement

**Breaking changes from v1.x:**
- Removed step-based calibration commands (REVCAL, BLCAL, etc.)
- Removed direction mode configuration (encoder determines optimal path)
- Added custom angle calibration system
- Changed DEVICE_ID format to ESP32FW-PID-V2.0
- Extended filter count support to 3-9 (was 3-8)

**Configuration Options (config.h):**
```cpp
// Display Power Management
#define DISPLAY_POWER_MODE DISPLAY_POWER_MODE_AUTO  // 0=auto, 1=always on, 2=always off
#define DISPLAY_AUTO_OFF_TIMEOUT 30     // Seconds (0=never, if mode is AUTO)

// Motor Performance
#define MOTOR_ACCELERATION 100000.0     // Instant acceleration (100,000 steps/s²)
#define MOTOR_SPEED 4000.0              // Operating speed (4000 steps/s)
#define MAX_MOTOR_SPEED 5000.0          // Maximum speed (5000 steps/s)

// Auto-Homing Behavior
#define AUTO_HOMING_ON_STARTUP true     // Enable auto-homing on startup
#define AUTO_HOMING_GO_TO_POSITION_1 false  // If true, always moves to position 1; if false, stays at current position if valid
```

For complete documentation, visit: https://juanjol.github.io/autoFilterWheel/