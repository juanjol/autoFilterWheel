# ASCOM Driver Command Reference

Complete command reference for the ESP32-C3 Filter Wheel Controller. All commands are prefixed with `#` and terminated with newline (`\n`).

## Command Format

- **Request**: `#COMMAND[parameters]\n`
- **Response**: `RESPONSE\n` (success) or `ERROR:message\n` (failure)
- **Baud Rate**: 115200
- **Line Ending**: LF (`\n`)

## Quick Command Reference

| Category | Commands |
|----------|----------|
| **Essential** | GP, MP, SP, GF, FC, STOP, STATUS |
| **System Info** | ID, VER, HELP |
| **Filter Names** | GN, SN |
| **Calibration** | CAL, CALSTART, CALCFM |
| **Custom Angles** | SETANG, GETANG, CLEARANG |
| **Encoder** | ENCSTATUS, ENCDIR, ENCRAW |
| **Motor Config** | GMC, MS, MXS, MA, MDD, RMC |
| **Direction** | MINV0, MINV1, GMINV, ENCINV0, ENCINV1, GENCINV |
| **Manual Control** | SF, SB, ME, MD, TESTMOTOR |
| **Display** | ROTATE, DISPLAY |

---

## Essential Commands (ASCOM Core)

### Get Current Position
**Command**: `#GP`
**Response**: `P[1-9]`
**Description**: Returns current filter position (1-based index)
**Example**:
```
Request:  #GP
Response: P3
```

### Move to Position
**Command**: `#MP[1-9]`
**Parameters**: Position number (1-9)
**Response**: `M[1-9]`
**Description**: Moves filter wheel to specified position. Blocks until movement complete.
**Control Mode**: Uses encoder-based PID control if available, falls back to step-based control
**Example**:
```
Request:  #MP2
Response: M2
```
**Note**: This is a **blocking command**. Response is sent only after movement completes.

### Get Filter Count
**Command**: `#GF`
**Response**: `F[3-9]`
**Description**: Returns number of filter positions
**Example**:
```
Request:  #GF
Response: F5
```

### Set Filter Count
**Command**: `#FC[3-9]`
**Parameters**: Number of filters (3-9)
**Response**: `FC[3-9]`
**Description**: Sets number of filter positions. Persists to EEPROM.
**Example**:
```
Request:  #FC8
Response: FC8
```

### Set Current Position
**Command**: `#SP[1-9]`
**Parameters**: Position number (1-9)
**Response**: `SP[1-9]`
**Description**: Manually sets current position without moving motor. Used for position synchronization.
**Example**:
```
Request:  #SP3
Response: SP3
```
**Warning**: Use with caution. Only use when physical position is known to match specified position.

### Emergency Stop
**Command**: `#STOP`
**Response**: `STOPPED`
**Description**: Immediately stops motor movement
**Example**:
```
Request:  #STOP
Response: STOPPED
```

---

## System Information

### Get Device ID
**Command**: `#ID`
**Response**: `ESP32FW-PID-V2.0`
**Description**: Returns device identifier
**Example**:
```
Request:  #ID
Response: ESP32FW-PID-V2.0
```

### Get Firmware Version
**Command**: `#VER`
**Response**: `2.0.0`
**Description**: Returns firmware version
**Example**:
```
Request:  #VER
Response: 2.0.0
```

### Get System Status
**Command**: `#STATUS`
**Response**: Multi-line status report
**Description**: Returns complete system status including position, encoder angle, control mode, and error state
**Example**:
```
Request:  #STATUS
Response: STATUS:
Position: 3/5
Encoder: OK (angle: 144.50°, error: 0.35°)
Control Mode: ENCODER-BASED
Motor: DISABLED
Calibrated: YES
Error: NONE
```

### Get Help
**Command**: `#HELP`
**Response**: Multi-line help text
**Description**: Returns list of available commands with brief descriptions
**Example**:
```
Request:  #HELP
Response: Available Commands:
#GP - Get current position
#MP[1-9] - Move to position
#GF - Get filter count
...
```

---

## Filter Name Management

### Get Filter Name
**Command**: `#GN` or `#GN[1-9]`
**Response**:
- `#GN`: Returns all filter names as comma-separated list: `NAMES:name1,name2,name3,...`
- `#GN[X]`: Returns single filter name for position X: `N[X]:name`
**Description**: Retrieves filter names from EEPROM (or defaults if not customized)
**Example**:
```
Request:  #GN
Response: NAMES:Luminance,Red,Green,Blue,H-Alpha

Request:  #GN2
Response: N2:Red
```
**Default Names** (when not customized): Luminance, Red, Green, Blue, H-Alpha, Filter 6, Filter 7, Filter 8

### Set Filter Name
**Command**: `#SN[1-9]:Name`
**Parameters**: Position number and name (max 15 characters)
**Response**: `SN[1-9]:Name`
**Description**: Sets filter name for specified position. Persists to EEPROM.
**Example**:
```
Request:  #SN1:Luminance
Response: SN1:Luminance

Request:  #SN5:H-Alpha
Response: SN5:H-Alpha
```

---

## Calibration

### Calibrate Home Position
**Command**: `#CAL`
**Response**: `CALIBRATED`
**Description**: Calibrates encoder offset so current physical position becomes position 1 (0°)
**Requirements**: AS5600 encoder must be connected and operational
**Example**:
```
Request:  #CAL
Response: CALIBRATED
```
**Note**: Only needed once during initial setup. Offset is stored in EEPROM.

### Start Guided Calibration
**Command**: `#CALSTART`
**Response**: `CALSTART:OK`
**Description**: Enters guided calibration mode. System prompts for manual positioning.
**Example**:
```
Request:  #CALSTART
Response: CALSTART:OK
```
**Note**: Use with `#CALCFM` to complete calibration after manual positioning.

### Confirm Guided Calibration
**Command**: `#CALCFM`
**Response**: `CALCFM:OK`
**Description**: Confirms guided calibration and saves encoder offset.
**Requirements**: Must be preceded by `#CALSTART` and manual positioning
**Example**:
```
Request:  #CALCFM
Response: CALCFM:OK
```

---

## Custom Angle Calibration

The system supports custom angle specification for each filter position, allowing compensation for non-uniform filter spacing or irregular wheel designs.

### Set Custom Angle
**Command**: `#SETANG[1-9]:[angle]`
**Parameters**:
- Position number (1-9)
- Angle in degrees (0.0-360.0)
**Response**: `SETANG[pos]:[angle]`
**Description**: Sets custom angle for specified filter position. Persists to EEPROM.
**Example**:
```
Request:  #SETANG1:0.0
Response: SETANG1:0.0

Request:  #SETANG2:68.5
Response: SETANG2:68.5

Request:  #SETANG3:142.3
Response: SETANG3:142.3
```
**Note**: When custom angles are set, the system uses them instead of uniform distribution (360°/N).

### Get Custom Angle
**Command**: `#GETANG` or `#GETANG[1-9]`
**Response**:
- `#GETANG`: Returns all custom angles: `ANGLES:0.0,68.5,142.3,210.0,285.0`
- `#GETANG[X]`: Returns angle for position X: `ANG[X]:[angle]` or `ANG[X]:NOT_SET`
**Description**: Retrieves custom angles from EEPROM
**Example**:
```
Request:  #GETANG
Response: ANGLES:0.0,68.5,142.3,210.0,285.0

Request:  #GETANG2
Response: ANG2:68.5
```

### Clear Custom Angles
**Command**: `#CLEARANG`
**Response**: `CLEARANG:OK`
**Description**: Clears all custom angle calibrations. System reverts to uniform distribution.
**Example**:
```
Request:  #CLEARANG
Response: CLEARANG:OK
```
**Note**: After clearing, position angles are calculated as: Position N = (N-1) × (360°/FilterCount)

---

## Encoder Commands

### Get Encoder Status
**Command**: `#ENCSTATUS`
**Response**: Multi-line encoder diagnostics
**Description**: Returns encoder availability, health, angle, and calibration status
**Example**:
```
Request:  #ENCSTATUS
Response: Encoder Status:
Available: YES
Angle: 144.50°
Offset: 12.30°
Magnet: OK (status: 0x20)
AGC: 128
Health: GOOD
```

### Get Encoder Direction
**Command**: `#ENCDIR`
**Response**: `DIR:[CW|CCW]`
**Description**: Returns last detected rotation direction
**Example**:
```
Request:  #ENCDIR
Response: DIR:CW
```

### Get Raw Encoder Data
**Command**: `#ENCRAW`
**Response**: Multi-line raw encoder data
**Description**: Returns raw encoder values for debugging
**Example**:
```
Request:  #ENCRAW
Response: Raw Encoder Data:
Raw Angle (0-4095): 2048
Angle (degrees): 180.00
Status Register: 0x20
AGC Value: 128
Magnitude: 1850
```

---

## Motor Control (Advanced)

### Get Motor Configuration
**Command**: `#GMC`
**Response**: `MOTOR_CONFIG:SPEED=[value],MAX_SPEED=[value],ACCEL=[value],DISABLE_DELAY=[value],STEPS_PER_REV=[value],MOTOR_INV=[0|1],ENC_INV=[0|1]`
**Description**: Returns current motor speed, acceleration, disable delay, and direction inversion status
**Example**:
```
Request:  #GMC
Response: MOTOR_CONFIG:SPEED=300,MAX_SPEED=500,ACCEL=200,DISABLE_DELAY=1000,STEPS_PER_REV=2048,MOTOR_INV=0,ENC_INV=0
```
**Fields**:
- `SPEED`: Current motor speed (steps/second)
- `MAX_SPEED`: Maximum speed limit
- `ACCEL`: Acceleration rate (steps/second²)
- `DISABLE_DELAY`: Time motor stays enabled after movement (milliseconds)
- `STEPS_PER_REV`: Steps per revolution (motor + gearbox)
- `MOTOR_INV`: Motor direction inversion (0=normal, 1=inverted)
- `ENC_INV`: Encoder direction inversion (0=normal, 1=inverted)

### Set Motor Speed
**Command**: `#MS[50-3000]`
**Parameters**: Speed in steps/second
**Response**: `MS[value]`
**Description**: Sets motor movement speed. Persists to EEPROM.
**Example**:
```
Request:  #MS400
Response: MS400
```

### Set Max Motor Speed
**Command**: `#MXS[100-5000]`
**Parameters**: Max speed in steps/second
**Response**: `MXS[value]`
**Description**: Sets maximum motor speed limit. Persists to EEPROM.
**Example**:
```
Request:  #MXS600
Response: MXS600
```

### Set Motor Acceleration
**Command**: `#MA[50-2000]`
**Parameters**: Acceleration in steps/second²
**Response**: `MA[value]`
**Description**: Sets motor acceleration rate. Persists to EEPROM.
**Example**:
```
Request:  #MA1200
Response: MA1200
```

### Set Motor Disable Delay
**Command**: `#MDD[500-10000]`
**Parameters**: Delay in milliseconds
**Response**: `MDD[value]`
**Description**: Sets time motor remains powered after movement. Persists to EEPROM.
**Example**:
```
Request:  #MDD2000
Response: MDD2000
```

### Reset Motor Configuration
**Command**: `#RMC`
**Response**: `RMC:OK`
**Description**: Resets motor configuration to factory defaults
**Example**:
```
Request:  #RMC
Response: RMC:OK
```

---

## Direction Inversion

The system allows runtime configuration of motor and encoder direction inversion. This is useful when the motor or encoder is physically mounted in reverse orientation.

### Set Motor Direction Normal
**Command**: `#MINV0`
**Response**: `MINV:Normal`
**Description**: Sets motor direction to normal (not inverted). Persists to EEPROM.
**Example**:
```
Request:  #MINV0
Response: MINV:Normal
```

### Set Motor Direction Inverted
**Command**: `#MINV1`
**Response**: `MINV:Inverted`
**Description**: Sets motor direction to inverted (reversed). Persists to EEPROM.
**Example**:
```
Request:  #MINV1
Response: MINV:Inverted
```

### Get Motor Direction Inversion
**Command**: `#GMINV`
**Response**: `GMINV:[0|1] ([Normal|Inverted])`
**Description**: Returns current motor direction inversion status
**Example**:
```
Request:  #GMINV
Response: GMINV:0 (Normal)
```

### Set Encoder Direction Normal
**Command**: `#ENCINV0`
**Response**: `ENCINV:Normal`
**Description**: Sets encoder direction to normal (not inverted). Persists to EEPROM.
**Requirements**: AS5600 encoder must be available
**Example**:
```
Request:  #ENCINV0
Response: ENCINV:Normal
```

### Set Encoder Direction Inverted
**Command**: `#ENCINV1`
**Response**: `ENCINV:Inverted`
**Description**: Sets encoder direction to inverted (360° - angle). Persists to EEPROM.
**Requirements**: AS5600 encoder must be available
**Example**:
```
Request:  #ENCINV1
Response: ENCINV:Inverted
```

### Get Encoder Direction Inversion
**Command**: `#GENCINV`
**Response**: `GENCINV:[0|1] ([Normal|Inverted])`
**Description**: Returns current encoder direction inversion status
**Requirements**: AS5600 encoder must be available
**Example**:
```
Request:  #GENCINV
Response: GENCINV:0 (Normal)
```

**Use Cases**:
- Motor mounted upside-down or in reverse orientation
- Encoder magnet rotates opposite to expected direction
- Hardware wiring causes reversed motion
- Simplifies physical installation without rewiring

**Note**: Direction settings are loaded automatically at startup and persist across power cycles.

---

## Manual Motor Control (Debugging)

### Step Forward
**Command**: `#SF[steps]`
**Parameters**: Number of steps (positive integer)
**Response**: `SF[steps]`
**Description**: Manually moves motor forward by specified steps
**Example**:
```
Request:  #SF100
Response: SF100
```

### Step Backward
**Command**: `#SB[steps]`
**Parameters**: Number of steps (positive integer)
**Response**: `SB[steps]`
**Description**: Manually moves motor backward by specified steps
**Example**:
```
Request:  #SB100
Response: SB100
```

### Enable Motor
**Command**: `#ME`
**Response**: `ME:ON`
**Description**: Manually enables motor (powers coils)
**Example**:
```
Request:  #ME
Response: ME:ON
```

### Disable Motor
**Command**: `#MD`
**Response**: `MD:OFF`
**Description**: Manually disables motor (unpowers coils)
**Example**:
```
Request:  #MD
Response: MD:OFF
```

### Test Motor
**Command**: `#TESTMOTOR`
**Response**: `TESTMOTOR:OK`
**Description**: Performs a brief motor test sequence (enable, small movement, disable)
**Example**:
```
Request:  #TESTMOTOR
Response: TESTMOTOR:OK
```
**Note**: Used for hardware verification and testing motor connections.

---

## Display Control

### Rotate Display
**Command**: `#ROTATE`
**Response**: `ROTATE:[0|1]`
**Description**: Toggles display rotation 180°. Persists to EEPROM.
**Example**:
```
Request:  #ROTATE
Response: ROTATE:1
```

### Get Display Information
**Command**: `#DISPLAY`
**Response**: Multi-line display status
**Description**: Returns display configuration and rotation state
**Example**:
```
Request:  #DISPLAY
Response: Display Status:
Type: 0.42" OLED (72x40)
Rotation: 180°
State: READY
```

---

## Error Handling

### Error Response Format
All errors follow the format: `ERROR:message`

**Common Errors**:
- `ERROR:Invalid command` - Command not recognized
- `ERROR:Invalid position` - Position out of range
- `ERROR:Invalid parameter` - Parameter value out of range
- `ERROR:Invalid format` - Command syntax incorrect
- `ERROR:System busy` - Motor already moving
- `ERROR:Movement failed` - Motor movement could not complete
- `ERROR:Encoder not available` - Encoder required but not detected

---

## ASCOM Implementation Notes

### Position Indexing
- **Firmware**: 1-based indexing (1, 2, 3, ..., N)
- **ASCOM**: 0-based indexing (0, 1, 2, ..., N-1)
- **Conversion**: `ASCOM_Position = Firmware_Position - 1`

### Movement Timing
The `#MP` command is **synchronous/blocking**:
- Command is sent: `#MP3`
- Firmware blocks during movement (PID loop executes)
- Response sent only when complete: `M3`
- ASCOM driver should wait for response before continuing

**Timeout Recommendations**:
- Normal movement: 10 seconds
- Maximum (full rotation): 20 seconds
- Consider motor speed and filter count

### Position Verification
After `#MP` completes, verify position with `#GP`:
```csharp
SendCommand("#MP3");      // Returns "M3" when done
var pos = SendCommand("#GP");  // Returns "P3"
if (pos != "P3") {
    // Handle position mismatch
}
```

### Encoder-Based vs Step-Based Control

**Encoder-Based Control** (preferred):
- Uses AS5600 magnetic encoder for absolute positioning
- PID controller achieves <0.8° accuracy
- Automatically corrects for missed steps and mechanical errors
- Requires encoder hardware

**Step-Based Control** (fallback):
- Used when encoder not available or fails
- Open-loop step counting
- No automatic error correction
- Less accurate but functional

Check control mode with `#STATUS` command.

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

### Recommended Command Sequence

**Initialization**:
1. Open serial port (115200, 8N1)
2. `#ID` - Verify device identifier
3. `#VER` - Check firmware version
4. `#GF` - Get filter count
5. `#GN` - Get filter names
6. `#GP` - Get current position
7. `#STATUS` - Check system health

**Movement**:
1. Check not busy: `#GP` or `#STATUS`
2. Send move command: `#MP[X]`
3. Wait for response (blocking)
4. Verify position: `#GP`

**Shutdown**:
1. Ensure not moving: `#STATUS`
2. Close serial port

---

## PID Control Parameters

The firmware uses these PID parameters for encoder-based control:

```cpp
Kp = 4.5    // Proportional gain
Ki = 0.01   // Integral gain
Kd = 0.3    // Derivative gain
Tolerance = 0.8°  // Target precision
Max Iterations = 30
Settling Time = 150ms per iteration
```

These parameters are tuned for the 28BYJ-48 stepper motor and cannot be changed via serial commands (requires firmware recompile).

---

## Debug Mode

Debug output can be enabled by setting `DEBUG_MODE 1` in `config.h` and recompiling firmware.

When enabled, movement commands produce verbose output:
```
[moveToPosition] Called with position: 2
[moveToPosition] Using ENCODER-BASED control
[moveToPosition] Target angle: 72.00°
[PID] Starting PID control to 72.00° (tolerance: 0.80°)
[PID] Iter 1: Angle=0.00° Err=72.00° | P=324.0 I=0.7 D=0.0 → 150 steps
[PID] Iter 2: Angle=26.40° Err=45.60° | P=205.2 I=1.3 D=-31.7 → 150 steps
...
[PID] ✓ TARGET REACHED!
[moveToPosition] Motor disabled (encoder-based control)
```

**Note**: Debug mode should be **disabled** (DEBUG_MODE 0) for ASCOM use to prevent console pollution.

---

## Example ASCOM Driver Pseudocode

```csharp
public class FilterWheel : IFilterWheelV2
{
    private SerialPort _serialPort;

    public void Connect()
    {
        _serialPort.Open();

        // Verify device
        string id = SendCommand("#ID");
        if (id != "ESP32FW-PID-V2.0")
            throw new DriverException("Invalid device");

        // Get configuration
        int filterCount = GetFilterCount();  // #GF
        LoadFilterNames();  // #GN
    }

    public void SetPosition(short position)
    {
        // Convert ASCOM 0-based to firmware 1-based
        int firmwarePos = position + 1;

        // Send move command (blocking)
        string response = SendCommand($"#MP{firmwarePos}", timeout: 10000);

        // Verify response
        if (response != $"M{firmwarePos}")
            throw new DriverException("Movement failed");

        // Double-check position
        VerifyPosition(position);
    }

    public short GetPosition()
    {
        string response = SendCommand("#GP");
        // Parse "P3" -> 3, convert to 0-based
        int firmwarePos = int.Parse(response.Substring(1));
        return (short)(firmwarePos - 1);
    }

    private string SendCommand(string command, int timeout = 5000)
    {
        _serialPort.WriteLine(command);
        string response = _serialPort.ReadLine(timeout);

        if (response.StartsWith("ERROR:"))
            throw new DriverException(response);

        return response.Trim();
    }
}
```

---

## Hardware Specifications

- **Microcontroller**: ESP32-C3 (160MHz, 320KB RAM, 4MB Flash)
- **Motor**: 28BYJ-48 stepper with ULN2003 driver
- **Encoder**: AS5600 12-bit magnetic encoder (I2C, 0x36)
- **Display**: 0.42" OLED SSD1306 (72x40 visible, I2C)
- **Steps per Revolution**: 2048 (with internal 64:1 gearbox)
- **Positioning Accuracy**: <0.8° with encoder, ±2-5° without

---

## Version History

- **v2.0.0**: Current version with PID encoder-based control
- **v1.0.0**: Initial release with encoder support
- **v0.0.1**: Development version

---

## Support

- **Documentation**: https://juanjol.github.io/autoFilterWheel/
- **Repository**: https://github.com/juanjol/autoFilterWheel
- **Issues**: https://github.com/juanjol/autoFilterWheel/issues
