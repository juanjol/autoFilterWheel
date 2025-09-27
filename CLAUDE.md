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
- **Position Sensing**: AS5600 I2C magnetic encoder (optional but recommended)
- **Display**: 0.42" OLED (72x40 visible area) with coordinate offset compensation
- **Motor Direction**: Configurable unidirectional (default) vs bidirectional movement
- **Power Management**: Automatic motor disable after movement to save energy
- **Filter Names**: Customizable per position (up to 10 characters each)

### Core System (`src/main.cpp`)
- **Position Tracking**: Real-time position updates during movement with EEPROM persistence
- **Serial Protocol**: ASCOM-compatible command set (`#MP1`, `#GP`, `#STATUS`, etc.)
- **Error Handling**: Comprehensive timeout and validation system
- **Display Management**: 3-line layout optimized for 0.42" OLED constraints
- **Power Management**: Automatic motor disable with complete pin control

### Critical Configuration Values

**Motor Configuration:**
- `MOTOR_DIRECTION_MODE 0`: Unidirectional movement (1→2→3→4→5→1)
- `MOTOR_REVERSE_DIRECTION true`: Default rotation direction is reversed
- `STEPS_PER_FILTER 409`: Calculated for 5 positions (72° each)
- `AUTO_DISABLE_MOTOR true`: Power saving enabled

**Display Positioning (0.42" OLED):**
- Y coordinates: 24, 36, 52 (positioned for bottom-portion display)
- X offset: `OLED_X_OFFSET` (30px) for horizontal centering
- Layout: Status / Position / Filter Name

**EEPROM Storage:**
- `0x00`: Calibration flag (0xAA when calibrated)
- `0x04`: AS5600 angle offset (float)
- `0x08`: Current position (uint8_t, auto-saved)
- `0x0C`: Filter names flag (0xBB when custom names stored)
- `0x10`: Reserved for filter position calibration data
- `0x20+`: Custom filter names (16 bytes each, up to 15 chars + null)

## Important Implementation Details

### Position Calculation During Movement
The system tracks real-time position during movement using:
```cpp
uint8_t calculatedPosition = (currentStep / STEPS_PER_FILTER) + 1;
if (calculatedPosition > NUM_FILTERS) calculatedPosition = ((calculatedPosition - 1) % NUM_FILTERS) + 1;
```
This was corrected from a complex formula that caused position display to lag by one step.

### Motor Power Management
- Motor automatically disables after `MOTOR_DISABLE_DELAY` (1000ms) to save power
- Complete pin control with multiple LOW attempts to prevent residual current/vibration
- Position is preserved in EEPROM across power cycles
- `forceMotorOff()` function ensures all control pins are driven LOW

### Display Coordinate System
The 0.42" OLED displays only a portion of the 128x64 buffer. All content must be positioned at y≥24 with x+OLED_X_OFFSET for proper visibility. Test on actual hardware as coordinate mapping varies by manufacturer.

### Unidirectional Movement Logic
For position changes like 1→5, the motor moves forward through 2,3,4,5 (4 steps total) rather than taking a shorter reverse path. This ensures consistent mechanical behavior and avoids backlash issues.

## Serial Commands for Testing

Essential commands for development/debugging:
- `#CAL` - Calibrate current position as position 1
- `#MP[1-X]` - Move to filter position (X = current filter count)
- `#GP` - Get current position
- `#STATUS` - Complete system status with position, calibration, angle
- `#GF` - Get current filter count
- `#FC[3-8]` - Set filter count (3-8 filters supported)
- `#GN` - Get all filter names
- `#GN[1-X]` - Get specific filter name
- `#SN[1-X]:Name` - Set filter name (e.g., `#SN1:Luminance`)
- `#SF[X]` - Manual step forward (useful for fine-tuning)
- `#SP[1-X]` - Set current position without moving
- `#ID` - Get device identifier
- `#VER` - Get firmware version
- `#STOP` - Emergency stop

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
3. Verify `STEPS_PER_FILTER` calculation remains correct
4. Update angle calculations for AS5600 if using encoder
5. Update EEPROM storage calculation if needed

### Modifying Display Layout
- Remember 0.42" OLED constraints (content at y≥24, x+OLED_X_OFFSET)
- Test with actual hardware as coordinate mapping varies by manufacturer
- Use `OLED_X_OFFSET` for horizontal positioning
- Display update limited to `DISPLAY_UPDATE_INTERVAL` (100ms) to prevent flicker

### Motor Direction Changes
- Modify `MOTOR_REVERSE_DIRECTION` for simple direction flip
- For bidirectional mode, change `MOTOR_DIRECTION_MODE` to 1
- Bidirectional mode chooses shortest path to target

### Power Supply Considerations
- ESP32-C3 can be powered via USB (5V)
- ULN2003 can accept 5V or 12V supply for higher torque
- External supply recommended for motor (minimum 1A capacity)
- All grounds must be connected together

### Calibration and Position Accuracy
- AS5600 encoder provides 12-bit resolution (4096 positions per revolution)
- Motor provides 2048 steps per revolution
- Use encoder feedback for position verification after movements
- Calibration stores angle offset in EEPROM for consistent startup

## Hardware Integration Notes

### AS5600 Encoder Setup
- Requires 3.3V supply (NOT 5V)
- Magnet must be 0.5-3mm from chip surface
- Use diametrically magnetized neodymium magnet (6mm diameter)
- I2C address is fixed at 0x36

### Motor Configuration
- 28BYJ-48 motor has internal 64:1 gearbox (2048 steps/rev)
- ULN2003 driver uses 4-wire control (IN1-IN4)
- Motor can run on 5V or 12V (higher voltage = more torque)
- Power management prevents overheating and saves energy

### Contributing Guidelines
- Test all changes with actual hardware setup
- Maintain ASCOM compatibility for serial protocol
- Document any new configuration options
- Follow existing code formatting and commenting style
- Update README.md for user-facing changes