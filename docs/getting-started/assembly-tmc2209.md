# TMC2209 + NEMA17 Assembly Guide

This guide covers assembling the professional-grade configuration using the TMC2209 silent stepper driver with NEMA17 bipolar stepper motor.

## Prerequisites

Before starting, ensure you have:
- ✅ Read the [Configuration Options](configuration-options.md) guide
- ✅ Intermediate electronics experience
- ✅ Gathered all [required components](hardware.md#tmc2209-configuration)
- ✅ Good soldering skills and proper tools
- ✅ Multimeter and oscilloscope (recommended)

## Wiring Diagram Reference

![TMC2209 Wiring Diagram](../diagrams/wiring-tmc2209.svg)

## Step-by-Step Assembly

### Step 1: Prepare the ESP32-C3 Board

1. **Inspect and test the board** as in the ULN2003 guide
2. **Verify UART capabilities**:
   - Ensure GPIO16 and GPIO17 are available
   - Test basic UART communication if possible

### Step 2: Prepare the TMC2209 Driver

1. **Inspect the TMC2209 board**:
   - Verify it's a genuine TMC2209 (not clone)
   - Check for proper soldering of the TMC2209 chip
   - Note the pin layout (varies by manufacturer)

2. **Configure driver settings**:
   - **Microstepping**: Default is usually 1/16
   - **Current setting**: Adjust via UART (will be done in software)
   - **Address pins**: Set if using multiple drivers

!!! warning "Driver Quality"
    TMC2209 clones may not support all features. For best results, use genuine Trinamic TMC2209 or quality boards like BTT TMC2209.

### Step 3: Control Signal Connections

Connect the ESP32-C3 to TMC2209 control pins:

| ESP32-C3 Pin | TMC2209 Pin | Function | Notes |
|--------------|-------------|----------|-------|
| **GPIO2** | STEP | Step pulse | Rising edge triggers step |
| **GPIO3** | DIR | Direction | HIGH/LOW for direction |
| **GPIO4** | EN | Enable | LOW to enable, HIGH to disable |
| **GPIO16** | PDN_UART | Power down/UART | For configuration |
| **GPIO17** | UART | UART communication | Bidirectional |

!!! tip "Signal Quality"
    - Use short wires (<15cm) for control signals
    - Twisted pair for STEP/DIR if longer runs needed
    - Keep away from motor power wires

### Step 4: Power Connections

#### Logic Power (3.3V/5V)
| ESP32-C3 Pin | TMC2209 Pin | Function |
|--------------|-------------|----------|
| **3.3V** | VCC_IO | Logic power |
| **5V** | VCC | Logic power (alternative) |
| **GND** | GND | Ground |

#### Motor Power (12-24V)
Connect your external power supply:
| Power Supply | TMC2209 Pin | Function |
|--------------|-------------|----------|
| **+12-24V** | VM | Motor power |
| **GND** | GND | Power ground |

!!! danger "Power Supply Requirements"
    - **Voltage**: 12-24V (higher voltage = better performance)
    - **Current**: Minimum 2A, 3A recommended for NEMA17
    - **Quality**: Use switching supply with low ripple
    - **Protection**: Fuse the motor power supply

### Step 5: Motor Connection

Connect the NEMA17 bipolar stepper:

#### Standard NEMA17 Wiring
| TMC2209 Pin | Motor Wire | Coil |
|-------------|------------|------|
| **2B** | Black | Coil 2 |
| **2A** | Green | Coil 2 |
| **1A** | Red | Coil 1 |
| **1B** | Blue | Coil 1 |

!!! note "Motor Wire Colors"
    Wire colors vary by manufacturer. Measure coil resistance to identify pairs:
    - Coil 1: Usually Red-Blue (1-3Ω resistance)
    - Coil 2: Usually Black-Green (1-3Ω resistance)
    - Between coils: Open circuit (infinite resistance)

### Step 6: Display and Encoder Connections

Connect I2C devices exactly as in the ULN2003 guide:
- OLED display on GPIO5 (SDA) and GPIO6 (SCL)
- AS5600 encoder (optional) on same I2C bus
- **Use 3.3V power only** for both devices

### Step 7: Advanced TMC2209 Configuration

#### Current Setting Calculation
```cpp
// Motor current = (Rsense * 2.5V) / (1.41 * 256)
// For 0.11Ω sense resistor and 800mA motor:
// Current setting = (800mA * 256 * 1.41) / (2.5V / 0.11Ω) = ~12

// This will be set via UART in firmware
```

#### Microstepping Configuration
```cpp
// Available options: 1, 2, 4, 8, 16, 32, 64, 128, 256
// 16 microsteps = 3200 steps per revolution
// 256 microsteps = 51200 steps per revolution (ultra-smooth)
```

### Step 8: Software Configuration

1. **Configure the firmware**:
   ```cpp
   // In src/config.h:
   #define MOTOR_DRIVER_TMC2209      // Enable TMC2209 support

   // Verify pin assignments:
   #define MOTOR_STEP_PIN 2          // Step pulse
   #define MOTOR_DIR_PIN 3           // Direction
   #define MOTOR_ENABLE_PIN 4        // Enable
   #define MOTOR_RX_PIN 16           // UART RX
   #define MOTOR_TX_PIN 17           // UART TX
   #define MOTOR_MICROSTEPS 16       // Microstepping
   #define MOTOR_CURRENT_MA 800      // Motor current
   ```

2. **Update motor parameters**:
   ```cpp
   // High-performance settings for TMC2209:
   #define MAX_MOTOR_SPEED 2000.0    // Much faster than ULN2003
   #define MOTOR_ACCELERATION 1000.0 // Higher acceleration
   #define MOTOR_SPEED 1000.0        // Normal operating speed
   ```

### Step 9: Initial Testing

1. **Power supply test**:
   ```bash
   # Check voltages with multimeter:
   # - 12-24V at TMC2209 VM pin
   # - 3.3V or 5V at TMC2209 VCC_IO
   # - No shorts between power rails
   ```

2. **UART communication test**:
   ```bash
   # Upload firmware and check serial output for:
   # - TMC2209 initialization messages
   # - UART communication status
   # - Driver configuration confirmation
   ```

3. **Motor movement test**:
   ```bash
   # Test basic movement:
   #GP          # Get position
   #SF100       # Step forward 100 steps
   #SB100       # Step backward 100 steps
   ```

### Step 10: TMC2209 Feature Configuration

#### Silent Operation (StealthChop)
```cpp
// Enable in firmware for near-silent operation
// Trade-off: Slightly reduced torque at high speeds
```

#### Stall Detection
```cpp
// Configure stallGuard for automatic stall detection
// Useful for automatic homing and protection
```

#### CoolStep (Current Reduction)
```cpp
// Automatically reduces current when motor load is low
// Improves efficiency and reduces heat
```

## Performance Tuning

### Speed Optimization
```cpp
// TMC2209 can handle much higher speeds:
#define MAX_MOTOR_SPEED 5000.0      // 5000 steps/second
#define MOTOR_ACCELERATION 2000.0   // Fast acceleration
```

### Precision Optimization
```cpp
// Use high microstepping for ultimate precision:
#define MOTOR_MICROSTEPS 64         // 12800 steps/revolution
// or even:
#define MOTOR_MICROSTEPS 256        // 51200 steps/revolution
```

### Power Optimization
```cpp
// Current reduction settings:
// - Hold current: 50% of run current when stationary
// - StealthChop: Enable for silent operation
// - CoolStep: Automatic current adjustment
```

## Troubleshooting

### No motor movement
- **Check power supply** - Verify 12-24V at VM pin
- **UART communication** - Verify TMC2209 responds to configuration
- **Enable signal** - Ensure EN pin is LOW to enable driver
- **Motor wiring** - Verify coil connections and polarity

### Noisy operation
- **Disable StealthChop** - May be unstable at certain speeds
- **Adjust microstepping** - Higher values generally quieter
- **Check power supply** - Ripple can cause noise
- **Motor mounting** - Mechanical resonance

### Overheating
- **Reduce current** - Lower MOTOR_CURRENT_MA setting
- **Improve cooling** - Add heatsink to TMC2209
- **Check voltage** - Higher voltage reduces heat for same power
- **Enable CoolStep** - Automatic current reduction

### UART communication failure
- **Check wiring** - Verify RX/TX connections
- **Baud rate** - Ensure correct UART settings
- **Driver address** - Verify if multiple drivers used
- **Power sequence** - Power TMC2209 before UART init

## Advanced Features

### Multiple TMC2209 Drivers
If using multiple filter wheels:
```cpp
// Set different UART addresses
// Configure each driver independently
// Coordinate movements between wheels
```

### Closed-Loop Operation
With AS5600 encoder:
```cpp
// Real-time position feedback
// Automatic error correction
// Stall detection and recovery
```

## Next Steps

After successful assembly:

1. **[Advanced Calibration →](../user-guide/calibration-advanced.md)** - TMC2209-specific calibration
2. **[Performance Tuning →](../user-guide/motor-tuning-tmc2209.md)** - Optimize for your application
3. **[ASCOM Integration →](../ascom/installation.md)** - Same as ULN2003 configuration

## Comparison with ULN2003

| Feature | ULN2003 | TMC2209 |
|---------|---------|---------|
| **Steps/revolution** | 2048 | 3200 (16μ) to 51200 (256μ) |
| **Max speed** | 500 steps/sec | 5000+ steps/sec |
| **Noise level** | Moderate | Near silent |
| **Power efficiency** | Poor | Excellent |
| **Features** | Basic | Advanced |
| **Setup complexity** | Simple | Advanced |

The TMC2209 configuration provides professional-grade performance with significantly enhanced capabilities over the basic ULN2003 setup.