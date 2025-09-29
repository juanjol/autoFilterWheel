# DIY ASCOM compatible Filter Wheel for astronomy

[![Documentation Status](https://img.shields.io/badge/docs-mkdocs-blue)](https://juanjol.github.io/autoFilterWheel/)
[![GitHub Release](https://img.shields.io/github/v/release/juanjol/autoFilterWheel)](https://github.com/juanjol/autoFilterWheel/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/github/actions/workflow/status/juanjol/autoFilterWheel/docs.yml)](https://github.com/juanjol/autoFilterWheel/actions)

A DIY, open-source astronomy filter wheel controller built on the ESP32-C3 microcontroller. Features integrated OLED display, stepper motor control, magnetic encoder feedback, and full ASCOM compatibility for seamless integration with popular astronomy software.

## **[Complete Documentation Work in progress](https://juanjol.github.io/autoFilterWheel/)**

This README provides a quick overview. **For comprehensive guides, tutorials, and references, visit our full documentation site.**

## Features

- **3-8-Position Filter Control** - Automated filter wheel positioning
- **0.42" OLED Display** - Real-time status and position display
- **AS5600 Magnetic Encoder** - Precise position feedback (optional)
- **Power Management** - Automatic motor disable for energy saving
- **Serial Protocol** - ASCOM-compatible command interface
- **Position Memory** - Remembers position through power cycles
- **Flexible Direction** - Unidirectional or bidirectional movement

## Hardware Requirements

### Core Components
- **ESP32-C3 with integrated OLED** (128x64 pixels, 0.42" visible area)
- **28BYJ-48 Stepper Motor** (5V version)
- **ULN2003 Driver Board** for motor control
- **AS5600 Magnetic Encoder** (optional but recommended)
- **Neodymium Magnet** (6mm diameter, diametrically magnetized)
- **5V Power Supply** (minimum 1A for motor)

### Tools Required
- Soldering iron and wire
- Small screwdriver set
- Multimeter (for troubleshooting)

## Wiring Connections

### ESP32-C3 → ULN2003 Driver

```
ESP32-C3         ULN2003 Driver
--------         --------------
GPIO2     →      IN1
GPIO3     →      IN2
GPIO4     →      IN3
GPIO10    →      IN4
5V        →      VCC (or external 12V)
GND       →      GND
```

### ULN2003 → 28BYJ-48 Motor

Connect the 5-pin JST connector directly from ULN2003 to motor. The wire colors should match:
- Blue, Pink, Yellow, Orange, Red (center)

### ESP32-C3 → AS5600 Encoder

```
ESP32-C3         AS5600
--------         ------
GPIO5 (SDA)  →   SDA
GPIO6 (SCL)  →   SCL
3.3V         →   VCC (Important: 3.3V only!)
GND          →   GND
```

### Power Supply

```
External 5V Supply     Components
------------------     ----------
(+) 5V  → ESP32-C3 5V pin
        → ULN2003 VCC

(-) GND → All GND connections
```

> **⚠️ Power Warning**: Do not power the motor from ESP32 USB. Use external 5V supply with at least 1A capacity.

## Installation

### 1. Software Setup

Install PlatformIO in VS Code:
```bash
# Install PlatformIO extension in VS Code
# Or use PlatformIO CLI
pip install platformio
```

### 2. Build and Upload

```bash
# Build the project
pio run

# Upload to ESP32-C3
pio run -t upload

# Monitor serial output
pio device monitor
```

### 3. Initial Calibration

1. Connect to serial monitor (115200 baud)
2. Position filter wheel at filter #1
3. Send calibration command: `#CAL`
4. Test movement: `#MP2` (move to position 2)
5. Verify position: `#GP` (get current position)

## Configuration

All settings are in `src/config.h`:

### Filter Names

The system supports **dynamic filter names** that can be configured via software like NINA:

**Runtime Configuration (Recommended)**:
- Set names via serial: `#SN1:Luminance`, `#SN2:Red`, etc.
- Names are stored in EEPROM and persist across reboots
- Maximum 15 characters per filter name
- Configurable from astronomy software without firmware changes

**Compile-time Defaults** (Fallback):
```cpp
#define FILTER_NAME_1 "Luminance"     // Default if no custom names set
#define FILTER_NAME_2 "Red"
#define FILTER_NAME_3 "Green"
#define FILTER_NAME_4 "Blue"
#define FILTER_NAME_5 "H-Alpha"
```

### Motor Settings
```cpp
#define MOTOR_DIRECTION_MODE 0        // 0=Unidirectional, 1=Bidirectional
#define MOTOR_REVERSE_DIRECTION true  // Reverse if motor turns wrong way
#define MOTOR_SPEED 300.0             // Steps per second
```

### Display Settings
```cpp
#define OLED_X_OFFSET 30              // Horizontal centering for 0.42\" OLED
#define DISPLAY_UPDATE_INTERVAL 100   // Update frequency (ms)
```

## Serial Commands

### Position Control

| Command | Description | Example | Response |
|---------|-------------|---------|-----------|
| `#GP` | Get current position | `#GP` | `P3` |
| `#MP[1-5]` | Move to position | `#MP2` | `M2` |
| `#SP[1-5]` | Set current position | `#SP1` | `S1` |

### Filter Configuration

| Command | Description | Example | Response |
|---------|-------------|---------|-----------|
| `#GF` | Get number of filters | `#GF` | `F5` |
| `#FC[3-8]` | Set filter count | `#FC6` | `FC6` |
| `#GN` | Get all filter names | `#GN` | `NAMES:Luminance,Red,Green,Blue,H-Alpha` |
| `#GN[1-X]` | Get specific filter name | `#GN2` | `N2:Red` |
| `#SN[1-X]:Name` | Set filter name | `#SN1:Luminance` | `SN1:Luminance` |

### System Commands

| Command | Description | Example | Response |
|---------|-------------|---------|-----------|
| `#CAL` | Calibrate home position | `#CAL` | `CALIBRATED` |
| `#STATUS` | Get system status | `#STATUS` | `STATUS:POS=1,MOVING=NO...` |
| `#ID` | Get device identifier | `#ID` | `DEVICE_ID:ESP32FW-5POS-V1.0` |
| `#VER` | Get firmware version | `#VER` | `VERSION:1.0.0` |
| `#STOP` | Emergency stop | `#STOP` | `STOPPED` |

### Manual Control

| Command | Description | Example | Response |
|---------|-------------|---------|-----------|
| `#SF[X]` | Step forward X steps | `#SF100` | `SF100` |
| `#SB[X]` | Step backward X steps | `#SB50` | `SB50` |
| `#ST[X]` | Go to absolute step | `#ST1024` | `ST1024` |

## Display Layout

The 0.42\" OLED shows three lines of information:

```
Line 1: [Status]          - READY/MOVING/ERROR
Line 2: [POS X]           - Current position (large text)
Line 3: [Filter Name]     - Name of current filter
```

## Technical Specifications

### Motor Specifications
- **28BYJ-48**: 2048 steps per revolution
- **Resolution**: 0.176° per step
- **Filter Separation**: 409.6 steps (72° each)
- **Power**: 5V, max 240mA

### Encoder Specifications
- **AS5600**: 12-bit magnetic encoder
- **Resolution**: 4096 positions per revolution (0.088°)
- **Interface**: I2C (address 0x36)
- **Supply**: 3.3V only

### Performance
- **Position Accuracy**: ±1 step with encoder feedback
- **Movement Speed**: Configurable up to 500 steps/second
- **Power Consumption**: <50mA standby, <300mA during movement

## Troubleshooting

### Motor Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Motor doesn't move | Insufficient power | Use external 5V/1A supply |
| Erratic movement | Loose connections | Check all wiring |
| Wrong direction | Direction setting | Change `MOTOR_REVERSE_DIRECTION` |

### Display Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Blank display | I2C address | Verify OLED_ADDRESS in config |
| Partial display | Coordinate offset | Adjust OLED_X_OFFSET |
| No text visible | Y coordinate issue | Content must be at y≥24 |

### Encoder Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| AS5600 not detected | Power/connections | Check 3.3V supply and I2C wires |
| Inaccurate readings | Magnet position | Center magnet 0.5-3mm from chip |
| Position drift | Magnetic interference | Use shielded cables |

## ASCOM Driver

For telescope software integration, an ASCOM FilterWheel driver is planned. The hardware is fully compatible with the ASCOM standard interface.

### Current Status
- Hardware ready for ASCOM integration
- Serial protocol compatible with ASCOM requirements
- Driver development in progress

## Hardware Assembly Tips

### Filter Wheel Mounting
1. **Bearing Support**: Use ball bearings for smooth rotation
2. **Magnet Placement**: Center the magnet exactly over AS5600 chip
3. **Balance**: Ensure wheel is mechanically balanced
4. **Coupling**: Use flexible coupling between motor and wheel

### Magnet Installation
- **Distance**: 0.5-3mm from AS5600 surface
- **Alignment**: Center magnet over the chip
- **Orientation**: Diametric magnetization required
- **Size**: 6mm diameter recommended

## Contributing

This project is open source and contributions are welcome:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly with actual hardware
5. Submit a pull request

### Development Notes
- Test all changes with real hardware
- Maintain ASCOM compatibility
- Follow existing code style
- Update documentation for any changes

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

- Original development by the DIY astronomy community
- Uses AccelStepper library by Mike McCauley
- AS5600 encoder support
- Adafruit display libraries

## Changelog

### Version 1.0.0
- Initial release
- 5-position filter wheel control
- AS5600 encoder support
- OLED display with status
- Power management features
- EEPROM position memory
- Serial command interface

---

**⭐ If this project helps your astronomy setup, please give it a star!**