# Basic Commands

These are the essential commands you'll use for everyday operation of your filter wheel controller.

## Position Control

### Get Current Position - `GP`

Returns the current filter position.

=== "Command"
    ```bash
    #GP
    ```

=== "Response"
    ```bash
    P3
    ```

=== "Example"
    ```bash
    → #GP
    ← P1
    # Filter wheel is currently at position 1
    ```

**Use Cases:**
- Check position before planning moves
- Verify movement completion
- Status monitoring in automation scripts

---

### Move to Position - `MP[X]`

Moves the filter wheel to the specified position.

=== "Command"
    ```bash
    #MP[position]
    ```

=== "Parameters"
    - **position**: Target position (1 to configured filter count)

=== "Response"
    ```bash
    M[position]
    ```

=== "Examples"
    ```bash
    → #MP3
    ← M3
    # Moves to position 3

    → #MP1
    ← M1
    # Moves to position 1 (Luminance filter)
    ```

**Movement Behavior:**
- **Unidirectional mode**: Always moves in one direction (e.g., 1→2→3→4→5→1)
- **Bidirectional mode**: Takes shortest path (e.g., 5→1 goes backward)
- Movement is **non-blocking** - command returns immediately
- Use `GP` to check when movement is complete

!!! tip "Movement Optimization"
    The controller automatically applies backlash compensation and uses your configured motor settings for optimal performance.

---

### Set Current Position - `SP[X]`

Sets the current position without moving the motor. Used for manual positioning or correction.

=== "Command"
    ```bash
    #SP[position]
    ```

=== "Parameters"
    - **position**: Position to set as current (1 to configured filter count)

=== "Response"
    ```bash
    S[position]
    ```

=== "Examples"
    ```bash
    → #SP1
    ← S1
    # Current position is now set to 1

    → #SP4
    ← S4
    # Current position is now set to 4
    ```

**When to Use:**
- After manual rotation of the filter wheel
- Correcting position after power-up
- Recovery from missed steps or errors
- Initial setup before calibration

⚠️ **Caution**: Only use this command when you're certain of the physical position. Incorrect use can cause positioning errors.

---

## System Status

### Get System Status - `STATUS`

Returns comprehensive system information in a single command.

=== "Command"
    ```bash
    #STATUS
    ```

=== "Response Format"
    ```bash
    STATUS:POS=<pos>,MOVING=<moving>,CAL=<calibrated>,ANGLE=<angle>,ERROR=<error>
    ```

=== "Response Fields"
    - **POS**: Current position (1-8)
    - **MOVING**: YES/NO - Is motor currently moving
    - **CAL**: YES/NO - Is system calibrated
    - **ANGLE**: Current encoder angle (if AS5600 available)
    - **ERROR**: Error code (0 = no error)

=== "Examples"
    ```bash
    → #STATUS
    ← STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=180.5,ERROR=0
    # At position 3, not moving, calibrated, encoder at 180.5°, no errors

    → #STATUS
    ← STATUS:POS=2,MOVING=YES,CAL=YES,ANGLE=90.2,ERROR=0
    # At position 2, currently moving, calibrated, no errors
    ```

**Use Cases:**
- Comprehensive system health check
- Automation scripts needing full status
- Troubleshooting positioning issues
- Verifying calibration state

---

## System Control

### Calibrate Home Position - `CAL`

Sets the current physical position as position 1 (home). This is essential for accurate positioning.

=== "Command"
    ```bash
    #CAL
    ```

=== "Response"
    ```bash
    CALIBRATED
    ```

=== "Example"
    ```bash
    → #CAL
    ← CALIBRATED
    # Current position is now set as position 1
    ```

**Calibration Process:**
1. Manually position the filter wheel to your desired "position 1"
2. Send the `#CAL` command
3. System sets current location as position 1
4. All other positions are calculated relative to this point
5. If AS5600 encoder is present, angle offset is also stored

!!! warning "Important"
    Perform calibration with the filter wheel at the exact position you want as "position 1". This becomes the reference for all other positions.

---

### Emergency Stop - `STOP`

Immediately stops all motor movement. Use for emergency situations.

=== "Command"
    ```bash
    #STOP
    ```

=== "Response"
    ```bash
    STOPPED
    ```

=== "Example"
    ```bash
    → #STOP
    ← STOPPED
    # All movement immediately halted
    ```

**When to Use:**
- Emergency situations
- Unexpected behavior during movement
- Testing and development
- Manual intervention needed

**Effects:**
- Motor stops immediately (no deceleration)
- Current movement command is abandoned
- Position may not be at exact filter location
- Use `GP` to check final position after stop

---

## Device Information

### Get Device Identifier - `ID`

Returns the device identification string for ASCOM driver verification.

=== "Command"
    ```bash
    #ID
    ```

=== "Response"
    ```bash
    DEVICE_ID:ESP32_FILTER_WHEEL_V1
    ```

=== "Example"
    ```bash
    → #ID
    ← DEVICE_ID:ESP32_FILTER_WHEEL_V1
    # Confirms device identity
    ```

---

### Get Firmware Version - `VER`

Returns the current firmware version.

=== "Command"
    ```bash
    #VER
    ```

=== "Response"
    ```bash
    VERSION:1.0.0
    ```

=== "Example"
    ```bash
    → #VER
    ← VERSION:1.0.0
    # Running firmware version 1.0.0
    ```

---

## Command Sequences

### Basic Operation Sequence

```bash
# 1. Check current status
#STATUS

# 2. Move to desired filter
#MP3

# 3. Verify position (wait for movement to complete)
#GP

# 4. Check system is ready
#STATUS
```

### Initial Setup Sequence

```bash
# 1. Identify device
#ID

# 2. Check firmware version
#VER

# 3. Set home position
#CAL

# 4. Verify calibration
#STATUS

# 5. Test movement
#MP2
#GP
```

### Recovery Sequence

```bash
# 1. Stop any current movement
#STOP

# 2. Check system status
#STATUS

# 3. Manually position wheel to known location
# (physical manipulation)

# 4. Set correct position
#SP1

# 5. Re-calibrate if needed
#CAL
```

---

**Next Steps:**
- [**Configuration Commands →**](configuration.md) - Customize your filter wheel settings
- [**Calibration Commands →**](calibration.md) - Achieve maximum precision
- [**Error Codes →**](errors.md) - Understand error messages