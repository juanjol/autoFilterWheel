# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an **ESP32-C3 Astronomy Filter Wheel Controller** - an open-source project for controlling automated filter wheels in astronomy applications. It features integrated OLED display, stepper motor control, magnetic encoder feedback, and ASCOM-compatible serial protocol for telescope software integration.

## Build Commands

```bash
# Build and upload to ESP32-C3
pio run -t upload

# Monitor serial output (115200 baud)
pio device monitor

# Build only (no upload)
pio run

# Clean build
pio run -t clean
```

## Key Architecture Components

### Hardware Configuration (`src/config.h`)
- **Motor Control**: 28BYJ-48 stepper (2048 steps/revolution) with ULN2003 driver
- **Position Sensing**: AS5600 I2C magnetic encoder (**required** for PID control)
- **Display**: 0.42" OLED (72x40 visible area) with coordinate offset compensation
- **Motor Direction**: Configurable unidirectional (default) vs bidirectional movement
- **Power Management**: Automatic motor disable after movement to save energy
- **Filter Names**: Customizable per position (up to 15 characters each)

### Core System (`src/main.cpp`)
- **Position Tracking**: Real-time position updates during movement with EEPROM persistence
- **Serial Protocol**: ASCOM-compatible command set (`#MP1`, `#GP`, `#STATUS`, etc.)
- **Error Handling**: Comprehensive timeout and validation system
- **Display Management**: 3-line layout optimized for 0.42" OLED constraints
- **Power Management**: Automatic motor disable with complete pin control

### Critical Configuration Values

**Motor Configuration:**
- `STEPS_PER_REVOLUTION 2150`: 28BYJ-48 with internal gearing
- `MAX_MOTOR_SPEED 430.0`: Maximum steps per second
- `MOTOR_SPEED 300.0`: Normal operating speed
- `AUTO_DISABLE_MOTOR true`: Power saving enabled
- `MOTOR_DISABLE_DELAY 1000`: Motor disables after 1 second

**Display Positioning (0.42" OLED):**
- Y coordinates: 24, 36, 52 (positioned for bottom-portion display)
- X offset: `OLED_X_OFFSET` (30px) for horizontal centering
- Layout: Status / Position / Filter Name

**EEPROM Storage:**
- `0x00`: Calibration flag (0xAA when calibrated)
- `0x04`: AS5600 angle offset (float)
- `0x08`: Current position (uint8_t, auto-saved)
- `0x0C`: Filter names flag (0xBB when custom names stored)
- `0x10`: Filter count (uint8_t)
- `0x11`: Custom angles flag (0xCA when custom angles stored)
- `0x12`: Custom angles array (36 bytes: 9 floats × 4 bytes each)
- `0x40+`: Custom filter names (16 bytes each, up to 15 chars + null)

## Important Implementation Details

### Position Control Architecture

**Encoder-Based Control with PID (Primary Mode)**:
When AS5600 encoder is available, the system uses a PID controller for smooth, precise positioning:
- Target position converted to angle using either:
  - **Custom angles** (if calibrated): Each position has a specific angle stored in EEPROM
  - **Uniform distribution** (default): Position 1=0°, Position 2=72°, Position 3=144°, etc.
- **PID Control Loop**: Calculates optimal motor steps based on:
  - **P (Proportional)**: Kp=4.5 - Main driving force proportional to error
  - **I (Integral)**: Ki=0.01 - Eliminates steady-state error
  - **D (Derivative)**: Kd=0.3 - Dampens oscillation and overshoot
- Output limits: 10-2000 steps per iteration
- Precision: < 0.8° tolerance (configurable via `ANGLE_CONTROL_TOLERANCE`)
- Maximum 30 iterations with 150ms settling time per iteration
- Auto-corrects for mechanical errors, backlash, and missed steps
- Bidirectional movement: Can reverse if overshoot occurs

**Step-Based Control (Fallback Mode)**:
When encoder is not available or encoder control fails:
```cpp
uint8_t calculatedPosition = (currentStep / STEPS_PER_FILTER) + 1;
if (calculatedPosition > NUM_FILTERS) calculatedPosition = ((calculatedPosition - 1) % NUM_FILTERS) + 1;
```
- Traditional open-loop step counting (2048 steps/revolution ÷ 5 positions = 409 steps/position)
- No feedback correction, relies on motor accuracy
- Used as fallback for compatibility

### Motor Power Management
- Motor automatically disables after `MOTOR_DISABLE_DELAY` (1000ms) to save power
- Complete pin control with multiple LOW attempts to prevent residual current/vibration
- Position is preserved in EEPROM across power cycles
- `forceMotorOff()` function ensures all control pins are driven LOW

### Display Coordinate System
The 0.42" OLED displays only a portion of the 128x64 buffer. All content must be positioned at y≥24 with x+OLED_X_OFFSET for proper visibility. Test on actual hardware as coordinate mapping varies by manufacturer.

## Serial Commands

**Essential Commands:**
- `#CAL` - Calibrate encoder offset (sets current position as position 1 = 0°)
- `#MP[1-X]` - Move to filter position using encoder feedback
- `#GP` - Get current position
- `#STATUS` - Complete system status with angle, error, and control mode
- `#GF` - Get current filter count
- `#FC[3-9]` - Set filter count (3-9 filters supported)
- `#GN` - Get all filter names
- `#GN[1-X]` - Get specific filter name
- `#SN[1-X]:Name` - Set filter name (e.g., `#SN1:Luminance`)
- `#ID` - Get device identifier
- `#VER` - Get firmware version
- `#STOP` - Emergency stop

**Custom Angle Calibration:**
- `#SETANG[pos]:[angle]` - Manually set custom angle for position (e.g., `#SETANG1:0.0`, `#SETANG2:68.5`)
- `#GETANG[pos]` - Get custom angle for specific position (or `#GETANG` for all)
- `#CLEARANG` - Clear all custom angles (revert to uniform distribution)

**Debug/Manual Control:**
- `#SF[X]` - Manual step forward (for calibration/testing)
- `#SB[X]` - Manual step backward (for calibration/testing)
- `#CALSTART` - Start guided calibration mode (for encoder offset)
- `#CALCFM` - Confirm guided calibration
- `#ENCSTATUS` - Get encoder diagnostics
- `#ME` / `#MD` - Enable/disable motor manually

**Hardware Control:**
- `#ROTATE` - Rotate display 180 degrees
- `#ENCSTATUS` - Get encoder diagnostics and health status

## Common Development Tasks

### Filter Name Configuration
The system supports dynamic filter names configurable at runtime:

**Dynamic Names (Recommended)**:
- Use `#SN[1-5]:Name` command to set filter names
- Names stored in EEPROM with magic byte 0xBB
- Loaded automatically on startup
- Maximum 15 characters per name
- Persistent across power cycles

**Static Names (Fallback)**:
- Defined in `FILTER_NAME_1` through `FILTER_NAME_5` constants
- Used when no custom names are stored in EEPROM
- Can be updated by recompiling firmware

### Adding New Filter Positions
1. Update `NUM_FILTERS` in config.h
2. Add corresponding `FILTER_NAME_X` definitions for defaults
3. Angles are calculated automatically: Position N = (N-1) × (360°/NUM_FILTERS)
4. No other changes needed - encoder handles the rest

### Modifying Display Layout
- Remember 0.42" OLED constraints (content at y≥24, x+OLED_X_OFFSET)
- Test with actual hardware as coordinate mapping varies by manufacturer
- Use `OLED_X_OFFSET` for horizontal positioning
- Display update limited to `DISPLAY_UPDATE_INTERVAL` (100ms) to prevent flicker

### Power Supply Considerations
- ESP32-C3 can be powered via USB (5V)
- ULN2003 can accept 5V or 12V supply for higher torque
- External supply recommended for motor (minimum 1A capacity)
- All grounds must be connected together

### Calibration and Position Accuracy

**Encoder Offset Calibration**:
- Use `#CAL` command to calibrate position 1 at current physical location
- System calculates and stores angle offset so current angle becomes 0°
- Offset stored in EEPROM for persistence across power cycles
- AS5600 provides 12-bit resolution (4096 counts/revolution = 0.088°/count)

**Custom Angle Calibration**:
Manual angle setting allows you to specify exact angles for each filter position:

```
#SETANG1:0.0             // Set position 1 to 0°
#SETANG2:68.5            // Set position 2 to 68.5°
#SETANG3:142.3           // Set position 3 to 142.3°
// ... etc
```
- Direct angle specification
- Useful when angles are already known or measured
- Each position can be set independently
- Use `#SF` and `#SB` commands to fine-tune wheel position before setting angles

**Why Use Custom Angles?**
- Compensate for non-uniform filter spacing in wheel
- Account for manufacturing tolerances
- Support irregular wheel designs
- Maximize positioning accuracy for specific hardware

**Checking Custom Angles**:
- `#GETANG` - Show all configured angles
- `#GETANG2` - Show angle for specific position
- System automatically uses custom angles when available
- Falls back to uniform distribution if not calibrated

**Angle-Based Positioning Accuracy**:
- Target precision: < 1° (configurable via `ANGLE_CONTROL_TOLERANCE`)
- Typical achieved accuracy: 0.5-0.8° after feedback loop convergence
- Motor provides 2048 steps/revolution (0.176°/step nominal)
- Closed-loop control compensates for mechanical imperfections automatically

**Step-Based Positioning (Fallback)**:
- Accuracy depends on motor: ±2-5° typical for 28BYJ-48
- No automatic correction for missed steps or mechanical errors
- Position errors accumulate over multiple movements

## Hardware Integration Notes

### AS5600 Encoder Setup
- Requires 3.3V supply (NOT 5V)
- Magnet must be 0.5-3mm from chip surface
- Use diametrically magnetized neodymium magnet (6mm diameter)
- I2C address is fixed at 0x36
- **Direction Configuration**: Set `AS5600_INVERT_DIRECTION` in config.h to match motor rotation

### Motor Configuration
- 28BYJ-48 motor has internal 64:1 gearbox (2048 steps/rev)
- ULN2003 driver uses 4-wire control (IN1-IN4)
- Motor can run on 5V or 12V (higher voltage = more torque)
- Power management prevents overheating and saves energy

### PID Tuning for Angle Control

The PID controller parameters are defined in [config.h](src/config.h#L169):

```cpp
#define ANGLE_PID_KP 4.5f          // Proportional gain
#define ANGLE_PID_KI 0.01f         // Integral gain
#define ANGLE_PID_KD 0.3f          // Derivative gain
#define ANGLE_PID_INTEGRAL_MAX 100.0f  // Anti-windup limit
#define ANGLE_PID_OUTPUT_MIN 10    // Minimum motor steps per iteration
#define ANGLE_PID_OUTPUT_MAX 2000  // Maximum motor steps per iteration
#define ANGLE_PID_SETTLING_TIME 150  // Delay between iterations (ms)
```

**Tuning Guide:**
- **Kp (Proportional)**: Increase for faster response, decrease if overshooting
  - Too high: Oscillation around target
  - Too low: Slow, sluggish response
  - Current value (4.5): Balanced response for 28BYJ-48

- **Ki (Integral)**: Eliminates steady-state error
  - Too high: Slow oscillation, instability
  - Too low: Position never settles exactly on target
  - Current value (0.01): Conservative to prevent windup

- **Kd (Derivative)**: Dampens oscillation
  - Too high: Sensitive to noise, jerky movement
  - Too low: Overshooting, ringing
  - Current value (0.3): Light damping to reduce noise sensitivity

**Debugging PID Performance:**
Monitor serial output during movement:
```
[PID] Iter 1: Angle=0.00° Err=72.00° | P=396.0 I=0.0 D=0.0 → 150 steps (26.4°)
[PID] Iter 2: Angle=27.15° Err=44.85° | P=246.7 I=0.9 D=-32.6 → 150 steps (26.4°)
[PID] Iter 3: Angle=54.32° Err=17.68° | P=97.2 I=2.3 D=-32.6 → 66 steps (11.6°)
[PID] Iter 4: Angle=71.45° Err=0.55° | P=3.0 I=2.7 D=-20.5 → 10 steps (1.8°)
[PID] ✓ TARGET REACHED!
```

**Common Issues:**
- **Oscillation**: Reduce Kp or increase Kd
- **Slow settling**: Increase Kp, check SETTLING_TIME
- **Overshoot**: Increase Kd or reduce OUTPUT_MAX
- **Never reaches target**: Increase Ki slightly, check OUTPUT_MIN

### Contributing Guidelines
- Test all changes with actual hardware setup
- Maintain ASCOM compatibility for serial protocol
- Document any new configuration options
- Follow existing code formatting and commenting style
- Update README.md for user-facing changes

### Git
- Commits must not have any Claude trace or text
- We use main branch for stable releases, develop for working
- We use dedicated branches for new features
