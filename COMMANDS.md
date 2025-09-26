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
| `GP` | Get current position | None | `#GP` | `P3` | Returns current filter position (1-8) |
| `MP[X]` | Move to position | X = Position (1-8) | `#MP2` | `M2` | Moves to specified filter position |
| `SP[X]` | Set current position | X = Position (1-8) | `#SP1` | `S1` | Sets current position without moving |

## Filter Configuration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `GF` | Get filter count | None | `#GF` | `F5` | Returns number of filters (3-8) |
| `FC[X]` | Set filter count | X = Count (3-8) | `#FC6` | `FC6` | Sets total number of filters |
| `GN` | Get all filter names | None | `#GN` | `NAMES:Luminance,Red,Green,Blue,H-Alpha` | Comma-separated list |
| `GN[X]` | Get specific filter name | X = Position (1-8) | `#GN2` | `N2:Red` | Returns name for position X |
| `SN[X]:Name` | Set filter name | X = Position, Name = Filter name | `#SN1:Luminance` | `SN1:Luminance` | Max 15 characters |

## System Information Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `STATUS` | Get system status | None | `#STATUS` | `STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=180.5,ERROR=0` | Complete system state |
| `ID` | Get device identifier | None | `#ID` | `DEVICE_ID:ESP32_FILTER_WHEEL_V1` | Device identification |
| `VER` | Get firmware version | None | `#VER` | `VERSION:1.0.0` | Current firmware version |
| `CAL` | Calibrate home position | None | `#CAL` | `CALIBRATED` | Sets current position as position 1 |
| `STOP` | Emergency stop | None | `#STOP` | `STOPPED` | Immediately stops all movement |

## Manual Stepping Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `SF[X]` | Step forward | X = Steps (1-1000) | `#SF100` | `SF100` | Manual forward stepping |
| `SB[X]` | Step backward | X = Steps (1-1000) | `#SB50` | `SB50` | Manual backward stepping |
| `ST[X]` | Step to absolute position | X = Absolute step | `#ST1024` | `ST1024` | Move to specific step count |
| `GST` | Get current step position | None | `#GST` | `STEP:1024` | Returns absolute step position |

## Motor Configuration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `MS[X]` | Set motor speed | X = Speed (50-3000) | `#MS1000` | `MS1000` | Steps per second |
| `MXS[X]` | Set maximum speed | X = Max speed (100-5000) | `#MXS2000` | `MXS2000` | Maximum steps per second |
| `MA[X]` | Set motor acceleration | X = Acceleration (50-2000) | `#MA800` | `MA800` | Steps per second squared |
| `MDD[X]` | Set motor disable delay | X = Delay (500-10000) | `#MDD2000` | `MDD2000` | Auto-disable delay in milliseconds |
| `GMC` | Get motor configuration | None | `#GMC` | `MOTOR_CONFIG:SPEED=1000,MAX_SPEED=2000,ACCELERATION=800,DISABLE_DELAY=1000` | All motor parameters |
| `RMC` | Reset motor config | None | `#RMC` | `MOTOR_CONFIG_RESET` | Restore default motor settings |

## Motor Direction Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `MDM[X]` | Set direction mode | X = Mode (0/1) | `#MDM1` | `MDM1` | 0=Unidirectional, 1=Bidirectional |
| `MRV[X]` | Set reverse mode | X = Reverse (0/1) | `#MRV1` | `MRV1` | 0=Normal, 1=Reversed |
| `GDC` | Get direction config | None | `#GDC` | `DIRECTION_CONFIG:MODE=0,REVERSE=1` | Current direction settings |

## Motor Power Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `ME` | Enable motor power | None | `#ME` | `MOTOR_ENABLED` | Manually enable motor |
| `MD` | Disable motor power | None | `#MD` | `MOTOR_DISABLED` | Manually disable motor |

## Revolution Calibration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `REVCAL` | Start revolution calibration | None | `#REVCAL` | `REV_CAL_STARTED:2048` | Begins full revolution calibration |
| `RCP[X]` | Add calibration steps | X = Steps (1-100) | `#RCP10` | `RCP10 TOTAL:2058` | Fine-tune revolution with forward steps |
| `RCM[X]` | Subtract calibration steps | X = Steps (1-100) | `#RCM5` | `RCM5 TOTAL:2053` | Fine-tune revolution with backward steps |
| `RCFIN` | Finish revolution calibration | None | `#RCFIN` | `REV_CAL_COMPLETE:2053` | Save calibrated steps per revolution |

## Backlash Calibration Commands

| Command | Description | Parameters | Example | Response | Notes |
|---------|-------------|------------|---------|----------|-------|
| `BLCAL` | Start backlash calibration | None | `#BLCAL` | `BACKLASH_CAL_STARTED` | Requires AS5600 encoder |
| `BLS[X]` | Backlash test step | X = Steps (1-50) | `#BLS5` | `BLS5 TOTAL:15` | Move in small increments during calibration |
| `BLM` | Mark movement detected | None | `#BLM` | `BLM_FORWARD:15` or `BLM_BACKWARD:12` | Mark when encoder detects movement |
| `BLFIN` | Finish backlash calibration | None | `#BLFIN` | `BACKLASH_CAL_COMPLETE:15` | Save calibrated backlash compensation |

## Command Workflows

### Basic Operation
1. `#CAL` - Calibrate home position
2. `#MP3` - Move to filter position 3
3. `#GP` - Verify current position

### Filter Setup
1. `#FC5` - Set filter count to 5
2. `#SN1:Luminance` - Set filter names
3. `#SN2:Red`, `#SN3:Green`, etc.
4. `#GN` - Verify all filter names

### Motor Tuning
1. `#MS800` - Set motor speed
2. `#MA600` - Set acceleration
3. `#MXS2000` - Set maximum speed
4. `#GMC` - Verify configuration

### Revolution Calibration
1. `#REVCAL` - Start calibration (motor makes full revolution)
2. `#RCP10` or `#RCM5` - Fine-tune as needed
3. `#RCFIN` - Save calibration

### Backlash Calibration (Requires AS5600 encoder)
1. `#BLCAL` - Start calibration
2. `#BLS5` - Small test steps until movement detected
3. `#BLM` - Mark forward direction complete
4. `#BLS5` - Test backward direction
5. `#BLM` - Mark backward direction complete
6. `#BLFIN` - Save calibration

## Error Responses

| Error | Description | Cause |
|-------|-------------|--------|
| `ERROR:UNKNOWN_COMMAND` | Command not recognized | Invalid command syntax |
| `ERROR:INVALID_POSITION` | Position out of range | Position not between 1 and filter count |
| `ERROR:INVALID_COUNT` | Filter count invalid | Count not between 3-8 |
| `ERROR:INVALID_FORMAT` | Command format wrong | Missing parameters or wrong syntax |
| `ERROR:INVALID_SPEED` | Speed out of range | Motor speed not in valid range |
| `ERROR:INVALID_MAX_SPEED` | Max speed out of range | Maximum speed not in valid range |
| `ERROR:INVALID_ACCELERATION` | Acceleration out of range | Acceleration not in valid range |
| `ERROR:INVALID_DELAY` | Delay out of range | Disable delay not in valid range |
| `ERROR:INVALID_DIRECTION_MODE` | Direction mode invalid | Mode not 0 or 1 |
| `ERROR:INVALID_REVERSE_MODE` | Reverse mode invalid | Reverse not 0 or 1 |
| `ERROR:NAME_TOO_LONG` | Filter name too long | Name exceeds 15 characters |
| `ERROR:ENCODER_REQUIRED_FOR_BACKLASH_CAL` | No encoder detected | AS5600 encoder required for backlash calibration |
| `ERROR:REV_CAL_ALREADY_ACTIVE` | Revolution calibration active | Cannot start new calibration while one is running |
| `ERROR:BACKLASH_CAL_ALREADY_ACTIVE` | Backlash calibration active | Cannot start new calibration while one is running |
| `ERROR:NO_ACTIVE_REV_CAL_OR_MOVING` | No active calibration | Command requires active revolution calibration |
| `ERROR:NO_ACTIVE_BACKLASH_CAL_OR_MOVING` | No active calibration | Command requires active backlash calibration |
| `ERROR:BACKLASH_CAL_NOT_READY` | Calibration not ready | Both directions must be calibrated before finishing |

## Parameter Ranges

| Parameter | Minimum | Maximum | Default | Unit |
|-----------|---------|---------|---------|------|
| Filter Count | 3 | 8 | 5 | filters |
| Filter Position | 1 | 8 | - | position |
| Motor Speed | 50 | 3000 | 1000 | steps/sec |
| Max Motor Speed | 100 | 5000 | 2000 | steps/sec |
| Motor Acceleration | 50 | 2000 | 500 | steps/sec² |
| Motor Disable Delay | 500 | 10000 | 1000 | milliseconds |
| Manual Steps | 1 | 1000 | - | steps |
| Revolution Adjust Steps | 1 | 100 | - | steps |
| Backlash Test Steps | 1 | 50 | - | steps |
| Filter Name Length | 1 | 15 | - | characters |
| Direction Mode | 0 | 1 | 0 | mode |
| Reverse Mode | 0 | 1 | 0 | boolean |

## EEPROM Storage

All configuration parameters are automatically saved to EEPROM and persist across power cycles:

- Filter count and names
- Motor configuration (speed, acceleration, etc.)
- Direction configuration (mode, reverse)
- Revolution calibration
- Backlash calibration
- Current position

## Debug Mode

When `DEBUG_MODE` is enabled in firmware, additional diagnostic information is sent to serial output. This includes:

- Detailed movement information
- Configuration change confirmations
- Error diagnostics
- Calibration progress
- Motor status updates

## ASCOM Integration

This command set is designed for ASCOM driver integration. Key ASCOM-compatible features:

- Position-based commands (1-indexed)
- Immediate response format
- Status reporting
- Error handling
- Device identification

## Hardware Requirements

- **Microcontroller**: ESP32-C3
- **Motor**: 28BYJ-48 stepper motor with ULN2003 driver
- **Display**: 0.42" OLED (SSD1306, 72x40 visible area)
- **Encoder**: AS5600 magnetic rotary encoder (optional, required for backlash calibration)
- **Power**: USB 5V or external 5V/12V for motor
- **Communication**: USB serial at 115200 baud

## Firmware Version

This command reference is for firmware version **1.0.0** and compatible versions.

For the latest firmware updates and documentation, visit: [Project Repository]