# Command Reference

The ESP32-C3 Filter Wheel Controller provides **47+ serial commands** for complete control and configuration. All commands use a simple text-based protocol over USB serial at 115200 baud.

## 🚀 Quick Start

### Connection Settings
```
Port: Auto-detect COM port
Baud Rate: 115200
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: None
Line Ending: LF (\n)
```

### Basic Command Format
```bash
#COMMAND[parameters]\n
```

- **Prefix**: `#` (required)
- **Command**: Case-sensitive command name
- **Parameters**: Command-specific values
- **Termination**: Line feed (`\n`)

### Essential Commands

```bash
#GP              # Get current position
#MP3             # Move to position 3
#STATUS          # Get complete system status
#CAL             # Calibrate encoder offset
#STOP            # Emergency stop
```

## 📚 Command Categories

### 🎯 Essential Operations
Core movement and status commands

| Command | Description | Example Response |
|---------|-------------|------------------|
| `#GP` | Get current position | `P3` |
| `#MP[1-9]` | Move to position | `M3` |
| `#SP[1-9]` | Set current position | `SP3` |
| `#GF` | Get filter count | `F5` |
| `#FC[3-9]` | Set filter count | `FC5` |
| `#STOP` | Emergency stop | `STOPPED` |
| `#STATUS` | System status | `STATUS:POS=3,MOVING=NO,...` |

### 📝 Filter Configuration
Manage filter names and positions

| Command | Description | Example |
|---------|-------------|---------|
| `#GN` | Get all filter names | `NAMES:Luminance,Red,Green,...` |
| `#GN[1-9]` | Get specific filter name | `N2:Red` |
| `#SN[1-9]:Name` | Set filter name | `#SN1:Luminance` |

### 🎯 Calibration & Custom Angles
Configure encoder and position angles

| Command | Description | Example |
|---------|-------------|---------|
| `#CAL` | Calibrate encoder offset | `CALIBRATED` |
| `#SETANG[1-9]:[angle]` | Set custom angle | `#SETANG1:0.0` |
| `#GETANG` | Get all custom angles | `ANGLES:0.0,68.5,142.3,...` |
| `#GETANG[1-9]` | Get specific angle | `ANG2:68.5` |
| `#CLEARANG` | Clear custom angles | `CLEARANG:OK` |

### ⚙️ Motor Configuration
Adjust motor speed, acceleration, and behavior

| Command | Description | Range |
|---------|-------------|-------|
| `#GMC` | Get motor configuration | - |
| `#MS[value]` | Set motor speed | 50-3000 steps/s |
| `#MXS[value]` | Set max motor speed | 100-5000 steps/s |
| `#MA[value]` | Set motor acceleration | 50-2000 steps/s² |
| `#MDD[value]` | Set motor disable delay | 500-10000 ms |
| `#RMC` | Reset motor config | - |

### 🔄 Direction Inversion
Configure motor and encoder direction

| Command | Description | Response |
|---------|-------------|----------|
| `#MINV0` | Motor direction normal | `MINV:Normal` |
| `#MINV1` | Motor direction inverted | `MINV:Inverted` |
| `#GMINV` | Get motor inversion | `GMINV:0 (Normal)` |
| `#ENCINV0` | Encoder direction normal | `ENCINV:Normal` |
| `#ENCINV1` | Encoder direction inverted | `ENCINV:Inverted` |
| `#GENCINV` | Get encoder inversion | `GENCINV:0 (Normal)` |

### 🔬 Encoder Diagnostics
Monitor and debug encoder

| Command | Description |
|---------|-------------|
| `#ENCSTATUS` | Complete encoder status |
| `#ENCDIR` | Rotation direction |
| `#ENCRAW` | Raw encoder data |

### 🎮 Manual Control
Direct motor control for testing

| Command | Description |
|---------|-------------|
| `#SF[steps]` | Step forward |
| `#SB[steps]` | Step backward |
| `#ME` | Enable motor |
| `#MD` | Disable motor |
| `#TESTMOTOR` | Test motor sequence |

### 📺 Display Control
Manage OLED display

| Command | Description |
|---------|-------------|
| `#ROTATE` | Toggle display rotation |
| `#DISPLAY` | Get display info |

### ℹ️ System Information
Device identification and help

| Command | Description | Response |
|---------|-------------|----------|
| `#ID` | Get device ID | `ESP32FW-PID-V2.0` |
| `#VER` | Get firmware version | `2.0.0` |
| `#HELP` | Show command list | Multi-line help |

## 🔄 Common Workflows

### Initial Setup
```bash
#CAL              # Calibrate encoder (position 1 = 0°)
#FC5              # Configure 5 filters
#SN1:Luminance    # Set filter names
#SN2:Red
#SN3:Green
#SN4:Blue
#SN5:H-Alpha
#STATUS           # Verify configuration
```

### Custom Angle Configuration
For non-uniform filter spacing:
```bash
#SETANG1:0.0      # Position 1 at 0°
#SETANG2:68.5     # Position 2 at 68.5°
#SETANG3:142.3    # Position 3 at 142.3°
#SETANG4:210.0    # Position 4 at 210.0°
#SETANG5:285.0    # Position 5 at 285.0°
#GETANG           # Verify angles
```

### Motor Tuning
```bash
#GMC              # Get current motor config
#MS400            # Set motor speed
#MA200            # Set acceleration
#MDD1000          # Set disable delay
```

### Direction Correction
```bash
#GMINV            # Check motor direction
#MINV1            # Invert if needed
#GENCINV          # Check encoder direction
#ENCINV1          # Invert if needed
```

## 📡 Response Format

### Success Response
```
Command: #MP3
Response: M3
```

### Error Response
```
Command: #MP10
Response: ERROR:Invalid position
```

### Status Response
```
Command: #STATUS
Response: STATUS:POS=3,MOVING=NO,CAL=YES,ERROR=0,ANGLE=144.50,TARGET_ANGLE=144.00,ANGLE_ERROR=0.50,CONTROL=ENCODER
```

## ⚠️ Common Errors

| Error | Meaning | Solution |
|-------|---------|----------|
| `ERROR:Invalid command` | Command not recognized | Check command spelling |
| `ERROR:Invalid position` | Position out of range | Use position 1-N (N=filter count) |
| `ERROR:Invalid parameter` | Parameter out of range | Check value ranges |
| `ERROR:System busy` | Motor moving | Wait for movement to complete |
| `ERROR:Encoder not available` | Encoder required but not detected | Check AS5600 connection |

## 📊 Performance

### PID Control Performance
- **Positioning Accuracy**: <0.8° (configurable via `ANGLE_CONTROL_TOLERANCE`)
- **Control Loop**: Maximum 30 iterations
- **Settling Time**: 150ms per iteration
- **Self-Correction**: Automatic error compensation

### Command Response Times
- **Basic queries**: < 1ms
- **Movement commands**: Blocking (waits for completion)
- **Configuration changes**: < 10ms (includes EEPROM write)

## 🔗 External References

**Complete Command Documentation**: [ASCOM_COMMANDS.md](https://github.com/juanjol/autoFilterWheel/blob/main/ASCOM_COMMANDS.md)

**Developer Guide**: [CLAUDE.md](https://github.com/juanjol/autoFilterWheel/blob/main/CLAUDE.md)

---

*For detailed command specifications, parameter ranges, and ASCOM integration examples, see [ASCOM_COMMANDS.md](https://github.com/juanjol/autoFilterWheel/blob/main/ASCOM_COMMANDS.md).*
