# ASCOM Serial Protocol Reference

This document describes the complete serial command protocol for the ESP32 Filter Wheel Controller, designed for ASCOM driver integration.

## Protocol Specifications

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None
- **Line Ending**: Commands are prefixed with `#` and terminated with newline (`\n`)

## Command Format

All commands follow this format:
```
#COMMAND[parameters]\n
```

## Response Format

All responses follow one of these formats:
- **Success**: `RESPONSE_CODE[data]` - Command executed successfully
- **Error**: `ERROR:[description]` - Command failed with error description

---

## Table of Contents

1. [Basic Position Commands](#basic-position-commands)
2. [System Information Commands](#system-information-commands)
3. [Filter Configuration Commands](#filter-configuration-commands)
4. [Calibration Commands](#calibration-commands)
5. [Custom Angle Commands](#custom-angle-commands)
6. [Encoder Commands](#encoder-commands)
7. [Motor Configuration Commands](#motor-configuration-commands)
8. [Direction Inversion Commands](#direction-inversion-commands)
9. [Manual Control Commands](#manual-control-commands)
10. [Display Commands](#display-commands)
11. [Error Codes](#error-codes)
12. [Debug Mode](#debug-mode)

---

## Basic Position Commands

### Get Position - #GP
**Description**: Get current filter position
**Format**: `#GP`
**Response**: `P[1-9]` where number is current position
**Example**:
```
Command:  #GP
Response: P3
```

### Move to Position - #MP[n]
**Description**: Move to specified filter position
**Format**: `#MP[position]` where position is 1-9
**Response**: `M[position]` on success
**Error**: `ERROR:Movement failed` if movement fails
**Example**:
```
Command:  #MP5
Response: M5
```
**Note**: This is a **blocking command** - response is sent only after movement completes.

### Set Position - #SP[n]
**Description**: Manually set current position without moving motor
**Format**: `#SP[position]` where position is 1-9
**Response**: `S[position]`
**Example**:
```
Command:  #SP1
Response: S1
```

### Emergency Stop - #STOP
**Description**: Immediately stop all motor movement
**Format**: `#STOP`
**Response**: `STOPPED`
**Example**:
```
Command:  #STOP
Response: STOPPED
```

---

## System Information Commands

### Get Device ID - #ID
**Description**: Get device identifier
**Format**: `#ID`
**Response**: `DEVICE_ID:ESP32_FILTER_WHEEL_V1`
**Example**:
```
Command:  #ID
Response: DEVICE_ID:ESP32_FILTER_WHEEL_V1
```

### Get Version - #VER
**Description**: Get firmware version
**Format**: `#VER`
**Response**: `VERSION:[version_string]`
**Example**:
```
Command:  #VER
Response: VERSION:2.0.0
```

### Get Status - #STATUS
**Description**: Get comprehensive system status
**Format**: `#STATUS`
**Response**: `STATUS:POS=[pos],MOVING=[YES|NO],CAL=[YES|NO],ANGLE=[angle],ERROR=[code]`
**Example**:
```
Command:  #STATUS
Response: STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=144.25,ERROR=0
```

**Response Fields**:
- `POS`: Current position (1-9)
- `MOVING`: YES if motor is moving, NO if stationary
- `CAL`: YES if encoder is calibrated, NO if not calibrated
- `ANGLE`: Current encoder angle in degrees (0-359.99) - only present if encoder available
- `ERROR`: Error code (0 = no error)

### Get Help - #HELP
**Description**: Get list of available commands
**Format**: `#HELP`
**Response**: Multi-line help text with command list
**Example**:
```
Command:  #HELP
Response: Available Commands:
#GP - Get current position
#MP[1-9] - Move to position
...
```

---

## Filter Configuration Commands

### Get Filter Count - #GF
**Description**: Get number of filter positions
**Format**: `#GF`
**Response**: `F[count]` where count is 3-9
**Example**:
```
Command:  #GF
Response: F5
```

### Set Filter Count - #FC[n]
**Description**: Set number of filter positions
**Format**: `#FC[count]` where count is 3-9
**Response**: `FC[count]`
**Error**: `ERROR:Count must be 3-9`
**Example**:
```
Command:  #FC7
Response: FC7
```

### Get Filter Name - #GN[n]
**Description**: Get filter name(s)
**Format**:
- `#GN` - Get all filter names
- `#GN[position]` - Get specific filter name (1-9)

**Response**:
- All names: `NAMES:[name1],[name2],...`
- Single name: `N[pos]:[name]`

**Example**:
```
Command:  #GN
Response: NAMES:Luminance,Red,Green,Blue,H-Alpha

Command:  #GN3
Response: N3:Green
```

### Set Filter Name - #SN[n]:[name]
**Description**: Set custom name for filter position
**Format**: `#SN[position]:[name]` where name is max 15 characters
**Response**: `SN[position]:[name]`
**Error**: `ERROR:Invalid parameter` if name too long
**Example**:
```
Command:  #SN1:Luminance
Response: SN1:Luminance
```

---

## Calibration Commands

### Calibrate Home - #CAL
**Description**: Calibrate encoder offset at current position (sets as position 1 = 0°)
**Format**: `#CAL`
**Response**: `CALIBRATED`
**Example**:
```
Command:  #CAL
Response: CALIBRATED
```

### Start Guided Calibration - #CALSTART
**Description**: Start guided calibration mode
**Format**: `#CALSTART`
**Response**: `CALSTART:OK`
**Example**:
```
Command:  #CALSTART
Response: CALSTART:OK
```

### Confirm Guided Calibration - #CALCFM
**Description**: Complete guided calibration
**Format**: `#CALCFM`
**Response**: `CALCFM:Complete`
**Example**:
```
Command:  #CALCFM
Response: CALCFM:Complete
```

---

## Custom Angle Commands

### Set Custom Angle - #SETANG[pos]:[angle]
**Description**: Set specific angle for a filter position
**Format**: `#SETANG[position]:[angle]` where angle is 0-359.99
**Response**: `SETANG:Position [pos] set to [angle]°`
**Error**: `ERROR:Invalid angle` if angle out of range
**Example**:
```
Command:  #SETANG2:68.5
Response: SETANG:Position 2 set to 68.50°
```
**Note**: Saves immediately to EEPROM, no additional save command needed.

### Get Custom Angle - #GETANG[n]
**Description**: Get custom angle(s)
**Format**:
- `#GETANG` - Get all custom angles
- `#GETANG[position]` - Get specific angle

**Response**:
- All angles: `GETANG:[1=0.00°,2=68.50°,3=142.30°,...]`
- Single angle: `GETANG[pos]:[angle]° (custom|default)`
- No custom angles: `GETANG:No custom angles configured (using uniform distribution)`

**Example**:
```
Command:  #GETANG
Response: GETANG:1=0.00°,2=68.50°,3=142.30°,4=215.00°,5=287.50°

Command:  #GETANG2
Response: GETANG2:68.50° (custom)
```

### Clear Custom Angles - #CLEARANG
**Description**: Remove all custom angles and revert to uniform distribution
**Format**: `#CLEARANG`
**Response**: `CLEARANG:All custom angles cleared. Using uniform distribution.`
**Example**:
```
Command:  #CLEARANG
Response: CLEARANG:All custom angles cleared. Using uniform distribution.
```

---

## Revolution Measurement

### Measure Revolution - #MEASREV
**Description**: Automatically measure steps for 360° rotation using encoder feedback
**Format**: `#MEASREV`
**Response**: `MEASREV:SUCCESS Steps=[count] Rotation=[degrees]° Update STEPS_PER_REVOLUTION to [count]`
**Error**: `MEASREV:INCOMPLETE Steps=[count] Rotation=[degrees]°` if measurement fails
**Example**:
```
Command:  #MEASREV
Response: MEASREV:SUCCESS Steps=42500 Rotation=360.00° Update STEPS_PER_REVOLUTION to 42500
```
**Note**: This command uses PID control with encoder feedback to complete exactly one 360° rotation, then reports the precise step count needed. Essential for calibrating motor configuration when mechanical setup changes.

---

## Encoder Commands

### Get Encoder Status - #ENCSTATUS
**Description**: Get detailed encoder diagnostics
**Format**: `#ENCSTATUS`
**Response**: `ENCSTATUS:Angle=[angle],Expected=[exp],Error=[err],Raw=[raw],Offset=[off],Dir=[CW|CCW|STOP],Health=[OK|FAULT]`
**Example**:
```
Command:  #ENCSTATUS
Response: ENCSTATUS:Angle=144.25,Expected=144.00,Error=0.25,Raw=2048,Offset=0.00,Dir=STOP,Health=OK
```
**Note**: Returns `ENCSTATUS:Not connected` if encoder is not available.

### Get Rotation Direction - #ENCDIR
**Description**: Get current encoder rotation direction
**Format**: `#ENCDIR`
**Response**: `ENCDIR:[CW (+1)|CCW (-1)|STOPPED (0)]`
**Example**:
```
Command:  #ENCDIR
Response: ENCDIR:STOPPED (0)
```

### Get Raw Encoder Data - #ENCRAW
**Description**: Get raw encoder debugging information
**Format**: `#ENCRAW`
**Response**: `ENCRAW:RawCounts=[counts],RawAngle=[angle],Offset=[off],Calculated=[calc],Actual=[actual]`
**Example**:
```
Command:  #ENCRAW
Response: ENCRAW:RawCounts=2048,RawAngle=180.00,Offset=0.00,Calculated=180.00,Actual=180.00
```

---

## Motor Configuration Commands

### Get Motor Configuration - #GMC
**Description**: Get all motor configuration parameters
**Format**: `#GMC`
**Response**: `MOTOR_CONFIG:SPEED=[speed],MAX_SPEED=[max],ACCEL=[accel],DISABLE_DELAY=[delay],STEPS_PER_REV=[steps],MOTOR_INV=[0|1],ENC_INV=[0|1]`
**Example**:
```
Command:  #GMC
Response: MOTOR_CONFIG:SPEED=4000,MAX_SPEED=5000,ACCEL=100000,DISABLE_DELAY=1000,STEPS_PER_REV=34600,MOTOR_INV=0,ENC_INV=0
```

### Set Motor Speed - #MS[n]
**Description**: Set normal operating speed
**Format**: `#MS[speed]` where speed is 50-3000 steps/sec
**Response**: `MS[speed]`
**Error**: `ERROR:Invalid parameter` if out of range
**Example**:
```
Command:  #MS4000
Response: MS4000
```

### Set Max Motor Speed - #MXS[n]
**Description**: Set maximum motor speed
**Format**: `#MXS[speed]` where speed is 100-5000 steps/sec
**Response**: `MXS[speed]`
**Error**: `ERROR:Invalid parameter` if out of range
**Example**:
```
Command:  #MXS5000
Response: MXS5000
```

### Set Motor Acceleration - #MA[n]
**Description**: Set motor acceleration
**Format**: `#MA[accel]` where accel is 50-2000 steps/sec²
**Response**: `MA[accel]`
**Error**: `ERROR:Invalid parameter` if out of range
**Example**:
```
Command:  #MA100000
Response: MA100000
```

### Set Motor Disable Delay - #MDD[n]
**Description**: Set delay before motor auto-disables after movement
**Format**: `#MDD[delay]` where delay is 500-10000 milliseconds
**Response**: `MDD[delay]`
**Error**: `ERROR:Invalid parameter` if out of range
**Example**:
```
Command:  #MDD1000
Response: MDD1000
```

### Reset Motor Configuration - #RMC
**Description**: Reset all motor settings to defaults
**Format**: `#RMC`
**Response**: `MOTOR_CONFIG_RESET`
**Example**:
```
Command:  #RMC
Response: MOTOR_CONFIG_RESET
```

---

## Direction Inversion Commands

### Set Motor Direction Normal - #MINV0
**Description**: Set motor direction to normal (not inverted)
**Format**: `#MINV0`
**Response**: `MINV:Normal`
**Example**:
```
Command:  #MINV0
Response: MINV:Normal
```

### Set Motor Direction Inverted - #MINV1
**Description**: Set motor direction to inverted
**Format**: `#MINV1`
**Response**: `MINV:Inverted`
**Example**:
```
Command:  #MINV1
Response: MINV:Inverted
```

### Get Motor Inversion Status - #GMINV
**Description**: Get current motor direction inversion status
**Format**: `#GMINV`
**Response**: `GMINV:[0 (Normal)|1 (Inverted)]`
**Example**:
```
Command:  #GMINV
Response: GMINV:0 (Normal)
```

### Set Encoder Direction Normal - #ENCINV0
**Description**: Set encoder direction to normal (not inverted)
**Format**: `#ENCINV0`
**Response**: `ENCINV:Normal`
**Example**:
```
Command:  #ENCINV0
Response: ENCINV:Normal
```

### Set Encoder Direction Inverted - #ENCINV1
**Description**: Set encoder direction to inverted
**Format**: `#ENCINV1`
**Response**: `ENCINV:Inverted`
**Example**:
```
Command:  #ENCINV1
Response: ENCINV:Inverted
```

### Get Encoder Inversion Status - #GENCINV
**Description**: Get current encoder direction inversion status
**Format**: `#GENCINV`
**Response**: `GENCINV:[0 (Normal)|1 (Inverted)]`
**Example**:
```
Command:  #GENCINV
Response: GENCINV:0 (Normal)
```

---

## Manual Control Commands

### Step Forward - #SF[n]
**Description**: Manually step motor forward
**Format**: `#SF[steps]` where steps is optional (default 1)
**Response**: `SF[steps]`
**Error**: `ERROR:System busy` if moving
**Example**:
```
Command:  #SF100
Response: SF100
```
**Note**: Blocking command. Waits until movement completes.

### Step Backward - #SB[n]
**Description**: Manually step motor backward
**Format**: `#SB[steps]` where steps is optional (default 1)
**Response**: `SB[steps]`
**Error**: `ERROR:System busy` if moving
**Example**:
```
Command:  #SB50
Response: SB50
```
**Note**: Blocking command. Waits until movement completes.

### Motor Enable - #ME
**Description**: Manually enable motor
**Format**: `#ME`
**Response**: `MOTOR_ENABLED`
**Example**:
```
Command:  #ME
Response: MOTOR_ENABLED
```

### Motor Disable - #MD
**Description**: Manually disable motor
**Format**: `#MD`
**Response**: `MOTOR_DISABLED`
**Example**:
```
Command:  #MD
Response: MOTOR_DISABLED
```

### Test Motor - #TESTMOTOR
**Description**: Run direct motor hardware test
**Format**: `#TESTMOTOR`
**Response**: `TESTMOTOR:Running direct pin test... Complete. Check LEDs and motor movement.`
**Example**:
```
Command:  #TESTMOTOR
Response: TESTMOTOR:Running direct pin test... Complete. Check LEDs and motor movement.
```
**Note**: Used for hardware debugging and verification.

---

## Display Commands

### Rotate Display - #ROTATE[n]
**Description**: Rotate display 180 degrees
**Format**: `#ROTATE[0|1]` where 0=normal, 1=180° (parameter optional - toggles if omitted)
**Response**: `ROTATE[0|1]`
**Example**:
```
Command:  #ROTATE1
Response: ROTATE1
```

### Set Display Mode - #DISPMODE[n]
**Description**: Set display mode
**Format**: `#DISPMODE[0|1]` where 0=minimal, 1=detailed (parameter optional - toggles if omitted)
**Response**: `DISPMODE[mode]:[Minimal|Detailed]`
**Example**:
```
Command:  #DISPMODE1
Response: DISPMODE1:Detailed
```

### Get Display Info - #DISPLAY
**Description**: Get display configuration
**Format**: `#DISPLAY`
**Response**: `DISPLAY:Size=[w]x[h],Rotation=[Normal|180°],Enabled=[Yes|No],Update=[ms]ms,Brightness=[0-255]`
**Example**:
```
Command:  #DISPLAY
Response: DISPLAY:Size=128x64,Rotation=Normal,Enabled=Yes,Update=100ms,Brightness=128
```

### Set Brightness - #BRIGHT[n]
**Description**: Set display brightness
**Format**: `#BRIGHT[value]` where value is 0-255 (if omitted, returns current brightness)
**Response**: `BRIGHT:[value]`
**Example**:
```
Command:  #BRIGHT200
Response: BRIGHT:200
```

### Display On - #DISPON
**Description**: Turn display on
**Format**: `#DISPON`
**Response**: `DISPON:OK`
**Example**:
```
Command:  #DISPON
Response: DISPON:OK
```

### Display Off - #DISPOFF
**Description**: Turn display off
**Format**: `#DISPOFF`
**Response**: `DISPOFF:OK`
**Example**:
```
Command:  #DISPOFF
Response: DISPOFF:OK
```

### Set Display Power Mode - #DISPPOWER[n]
**Description**: Set display power management mode
**Format**: `#DISPPOWER[mode]` where mode is:
- 0 = Auto (uses timeout setting)
- 1 = Always on (never auto-off)
- 2 = Always off (manual on/off only)

If parameter omitted, returns current mode
**Response**: `DISPPOWER:[mode]:[Auto|AlwaysOn|AlwaysOff]`
**Example**:
```
Command:  #DISPPOWER0
Response: DISPPOWER:0:Auto

Command:  #DISPPOWER
Response: DISPPOWER:0:Auto
```

### Set Display Auto-Off Timeout - #DISPTIMEOUT[n]
**Description**: Set automatic display turn-off timeout (only applies in Auto power mode)
**Format**: `#DISPTIMEOUT[seconds]` where seconds is 0-65535
- 0 = Never auto-off (while in Auto mode)
- >0 = Seconds of inactivity before auto-off

If parameter omitted, returns current timeout
**Response**: `DISPTIMEOUT:[seconds]:[Seconds|Never]`
**Example**:
```
Command:  #DISPTIMEOUT60
Response: DISPTIMEOUT:60:Seconds

Command:  #DISPTIMEOUT0
Response: DISPTIMEOUT:0:Never

Command:  #DISPTIMEOUT
Response: DISPTIMEOUT:30:Seconds
```
**Note**: This setting only has effect when `#DISPPOWER0` (Auto mode) is active. In AlwaysOn or AlwaysOff modes, timeout is ignored.

---

## Error Codes

Common error responses and their meanings:

- `ERROR:System busy` - Motor is currently moving, command cannot execute
- `ERROR:Invalid format` - Command format is incorrect
- `ERROR:Invalid parameter` - Parameter is out of valid range
- `ERROR:Encoder not available` - AS5600 encoder is not connected or responding
- `ERROR:Motor driver not available` - Motor driver initialization failed
- `ERROR:Config manager not available` - Configuration system unavailable
- `ERROR:No controller` - Filter wheel controller not initialized
- `ERROR:Movement failed` - Motor movement command failed to execute

---

## Debug Mode

When `DEBUG_MODE` is enabled in `config.h`, additional diagnostic messages are sent with `[TAG]` prefixes:
- `[SETANG]` - Custom angle calibration messages
- `[CLEARANG]` - Angle clearing messages
- `[MEASREV]` - Revolution measurement progress
- `[MINV]` - Motor inversion messages
- `[ENCINV]` - Encoder inversion messages
- `[PID]` - PID controller iteration data

**Note**: ASCOM drivers should ignore messages starting with `[` as they are debug output only.

Debug mode can be controlled in firmware compilation:
```cpp
#define DEBUG_MODE 0  // 0=disabled, 1=enabled
```

---

## Typical ASCOM Driver Usage Sequence

1. **Initialize Connection**:
   ```
   #ID        → Verify device
   #VER       → Check firmware version
   #STATUS    → Get initial state
   ```

2. **Get Configuration**:
   ```
   #GF        → Get filter count
   #GN        → Get all filter names
   ```

3. **Move Filter**:
   ```
   #STATUS    → Check if ready (MOVING=NO)
   #MP3       → Move to position 3
   #STATUS    → Poll until MOVING=NO
   #GP        → Verify position
   ```

4. **Emergency Stop**:
   ```
   #STOP      → Stop immediately
   ```

---

## PID Control System

The firmware uses a closed-loop PID controller for encoder-based positioning:

**Parameters**:
- **Kp** = 4.5 (Proportional gain)
- **Ki** = 0.01 (Integral gain)
- **Kd** = 0.3 (Derivative gain)
- **Tolerance** = 0.8° (Target precision)
- **Max Iterations** = 30
- **Settling Time** = 150ms per iteration

**Performance**:
- Positioning accuracy: < 0.8° with encoder
- Fallback to step-based control (±2-5°) when encoder unavailable
- Automatic correction for backlash and missed steps
- Bidirectional movement optimization

---

## Custom Angle Calibration Workflow

For non-uniform filter wheel designs:

1. **Check Current Position**: `#GP` and `#ENCSTATUS`
2. **Fine-Tune Position**: Use `#SF` and `#SB` to adjust wheel manually
3. **Read Current Angle**: `#ENCSTATUS` to verify angle
4. **Save Angle**: `#SETANG[pos]:[angle]` to store custom angle
5. **Repeat**: For each filter position
6. **Verify**: `#GETANG` to confirm all angles saved
7. **Test Movement**: `#MP[pos]` to verify accurate positioning

To revert to uniform distribution: `#CLEARANG`

---

## Compliance Notes

This protocol is designed for ASCOM FilterWheel interface compliance. The following ASCOM properties map to these commands:

- `Position` (get) → `#GP`
- `Position` (set) → `#MP[n]`
- `Names` (get) → `#GN`
- `FocusOffsets` → Not implemented (always returns zeros)
- `Connected` → Serial port connection status
- `Description` → `#ID`
- `DriverInfo` → `#VER`
- `Name` → Device name from `#ID`

For complete ASCOM FilterWheel specification, see:
https://ascom-standards.org/Developer/FilterWheel.htm

---

## Hardware Reference

- **Microcontroller**: ESP32-C3 (160MHz, 320KB RAM, 4MB Flash)
- **Motor**: 28BYJ-48 stepper with ULN2003 driver
- **Encoder**: AS5600 12-bit magnetic encoder (I2C, 0x36)
- **Display**: 0.42" OLED SSD1306 (72x40 visible, I2C)
- **Steps per Revolution**: 2048 (default) - configurable with `#MEASREV`
- **Positioning Accuracy**: <0.8° with encoder, ±2-5° without

---

## Version History

- **v2.0.0**: Current version with PID encoder-based control and custom angle calibration
- **v1.0.0**: Initial release with encoder support
