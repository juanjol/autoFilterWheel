# Command Reference Overview

The ESP32-C3 Filter Wheel Controller provides a comprehensive set of serial commands for complete control and configuration. All commands use a simple text-based protocol over USB serial at 115200 baud.

## 🚀 Quick Start

### Basic Command Format

Commands follow a simple pattern:
```bash
#COMMAND[parameters]
```

- **Prefix**: `#` (optional but recommended)
- **Command**: Case-insensitive command name
- **Parameters**: Command-specific values in brackets or after colon
- **Termination**: Newline (`\n`) or carriage return (`\r`)

### Essential Commands

Get started with these fundamental commands:

```bash
#GP           # Get current position
#MP3          # Move to position 3
#STATUS       # Get complete system status
#CAL          # Calibrate home position
#STOP         # Emergency stop
```

## 📚 Command Categories

The controller supports over **50 commands** organized into logical categories:

### 🎯 [Basic Commands](basic.md)
**Core movement and status commands**

| Command | Description | Example |
|---------|-------------|---------|
| `GP` | Get current position | `#GP` → `P3` |
| `MP[X]` | Move to position | `#MP2` → `M2` |
| `SP[X]` | Set position | `#SP1` → `S1` |
| `STATUS` | System status | `#STATUS` → `STATUS:POS=3,MOVING=NO...` |

[View All Basic Commands →](basic.md){ .md-button }

### ⚙️ Configuration Commands
**System setup and parameter adjustment**

| Category | Commands | Examples |
|----------|----------|----------|
| **Filter Setup** | `FC`, `GF`, `SN`, `GN` | `#FC5`, `#SN1:Luminance` |
| **Motor Config** | `MS`, `MXS`, `MA`, `MDD` | `#MS1000`, `#MA800` |
| **Direction** | `MDM`, `MRV`, `GDC` | `#MDM1`, `#MRV0` |

### 🔧 Calibration Commands
**Precision calibration and tuning**

| Type | Commands | Purpose |
|------|----------|---------|
| **Revolution** | `REVCAL`, `RCP`, `RCM`, `RCFIN` | Calibrate steps per revolution |
| **Backlash** | `BLCAL`, `BLS`, `BLM`, `BLFIN` | Compensate mechanical play |
| **Position** | `CAL` | Set home reference |

### 🔬 Advanced Commands
**Manual control and diagnostics**

| Function | Commands | Use Cases |
|----------|----------|-----------|
| **Manual Steps** | `SF`, `SB`, `ST`, `GST` | Fine positioning, testing |
| **Motor Power** | `ME`, `MD` | Manual power control |
| **System Info** | `ID`, `VER` | Device identification |

## 🔄 Common Workflows

### First Time Setup
```bash
#CAL              # Set home position
#FC5              # Configure 5 filters
#SN1:Luminance    # Set filter names
#SN2:Red
#SN3:Green
#SN4:Blue
#SN5:H-Alpha
#STATUS           # Verify configuration
```

### Motor Tuning
```bash
#GMC              # Get current motor config
#MS800            # Set motor speed
#MA600            # Set acceleration
#MXS2000          # Set max speed
#MDD2000          # Set disable delay
#RMC              # Reset to defaults if needed
```

### Precision Calibration
```bash
#REVCAL           # Start revolution calibration
#RCP10            # Add 10 steps if needed
#RCFIN            # Finish and save

#BLCAL            # Start backlash calibration
#BLS5             # Test with 5-step increments
#BLM              # Mark movement detected
#BLFIN            # Complete calibration
```

## 📡 Communication Protocol

### Serial Settings
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None

### Response Format

All commands provide immediate responses:

=== "Success Response"
    ```bash
    Command: #MP3
    Response: M3
    ```

=== "Error Response"
    ```bash
    Command: #MP9
    Response: ERROR:INVALID_POSITION
    ```

=== "Status Response"
    ```bash
    Command: #STATUS
    Response: STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=180.5,ERROR=0
    ```

### Debug Mode

When `DEBUG_MODE` is enabled in firmware, additional diagnostic information is provided:

```bash
Command: #MP3
Response: M3
Debug:   Moving from #1 (Luminance) to #3 (Green) - 818 steps = 2.00 positions, UNIDIRECTIONAL
Debug:   Position reached: 3
```

## ⚠️ Error Handling

The controller provides comprehensive error reporting:

### Common Error Types
- `ERROR:UNKNOWN_COMMAND` - Invalid command syntax
- `ERROR:INVALID_POSITION` - Position out of range
- `ERROR:INVALID_FORMAT` - Wrong parameter format
- `ERROR:MOTOR_TIMEOUT` - Movement took too long


## 🛡️ Safety Features

### Movement Safety
- **Position validation** prevents invalid moves
- **Timeout protection** stops runaway movements
- **Emergency stop** command immediately halts motion
- **Power management** prevents motor overheating

### Data Safety
- **EEPROM storage** preserves settings through power cycles
- **Validation checks** prevent invalid configurations
- **Graceful error recovery** maintains system stability

## 🔗 Integration Examples

### Terminal/Command Line
```bash
# Windows
echo #GP > COM3
mode COM3 BAUD=115200 PARITY=n DATA=8

# Linux/Mac
echo "#GP" > /dev/ttyUSB0
stty -F /dev/ttyUSB0 115200
```

### Python Integration
```python
import serial

ser = serial.Serial('COM3', 115200, timeout=1)
ser.write(b'#GP\n')
response = ser.readline().decode().strip()
print(f"Current position: {response}")
```

### ASCOM Driver
The commands are wrapped by the ASCOM driver for seamless integration with astronomy software like NINA, PHD2, and others.

## 📊 Performance Characteristics

### Command Response Times
- **Basic commands**: < 1ms
- **Movement commands**: Immediate acknowledgment
- **Status queries**: < 5ms
- **Configuration changes**: < 10ms (includes EEPROM write)

### Movement Specifications
- **Position accuracy**: ±0.1 degrees with encoder
- **Repeatability**: ±0.05 degrees
- **Movement speed**: 50-3000 steps/second (configurable)
- **Acceleration**: 50-2000 steps/second² (configurable)

---

**Ready to explore specific commands?** Choose a category:

- [**Basic Commands →**](basic.md) - Start here for essential operations