# TMC2130 Connection Diagram for ESP32-C3 OLED Filter Wheel Controller

## Pin Connections

### ESP32-C3 OLED Board to TMC2130

```
ESP32-C3 OLED          TMC2130 Driver
--------------         --------------
GPIO 2  (STEP)    -->  STEP
GPIO 3  (DIR)     -->  DIR
GPIO 4  (EN)      -->  EN (Enable)
GPIO 10 (CS)      -->  CS (SPI Chip Select)

GPIO 18 (SCK)     -->  SCK (SPI Clock) - Hardware SPI
GPIO 19 (MOSI)    -->  SDI (SPI Data In)
GPIO 20 (MISO)    -->  SDO (SPI Data Out)

GPIO 5  (SDA)     -->  I2C OLED/AS5600
GPIO 6  (SCL)     -->  I2C OLED/AS5600

3.3V              -->  VIO (Logic voltage)
GND               -->  GND (Logic ground)
```

### Power Connections

```
Motor Power Supply     TMC2130 Driver
-----------------     --------------
12-24V DC (+)     -->  VM (Motor voltage)
GND (-)           -->  GND (Motor ground)

TMC2130           Stepper Motor (NEMA 17)
-------           ---------------------
A1                -->  Motor Coil A+
A2                -->  Motor Coil A-
B1                -->  Motor Coil B+
B2                -->  Motor Coil B-
```

### Complete Wiring Diagram

```
                ESP32-C3 OLED Board
                ┌─────────────────┐
                │                 │
                │  GPIO2 ─────────┼────> STEP (TMC2130)
                │  GPIO3 ─────────┼────> DIR  (TMC2130)
                │  GPIO4 ─────────┼────> EN   (TMC2130)
                │  GPIO10 ────────┼────> CS   (TMC2130)
                │                 │
                │  GPIO18 (SCK) ──┼────> SCK  (TMC2130)
                │  GPIO19 (MOSI)──┼────> SDI  (TMC2130)
                │  GPIO20 (MISO)──┼────> SDO  (TMC2130)
                │                 │
                │  GPIO5 (SDA) ───┼────> SDA (OLED & AS5600)
                │  GPIO6 (SCL) ───┼────> SCL (OLED & AS5600)
                │                 │
                │  3.3V ──────────┼────> VIO (TMC2130)
                │  GND ───────────┼────> GND (TMC2130)
                │                 │
                │  USB-C Power    │
                └─────────────────┘

                TMC2130 SilentStepStick
                ┌─────────────────┐
                │                 │
     STEP <─────┤ STEP       VM  ├────< 12-24V DC
      DIR <─────┤ DIR        GND ├────< Power GND
       EN <─────┤ EN         A1  ├────> Motor A+
       CS <─────┤ CS         A2  ├────> Motor A-
      SCK <─────┤ SCK        B1  ├────> Motor B+
      SDI <─────┤ SDI        B2  ├────> Motor B-
      SDO <─────┤ SDO            │
      VIO <─────┤ VIO            │
      GND <─────┤ GND            │
                │                 │
                │ [Jumper Config] │
                │ SPI Mode: Set   │
                └─────────────────┘

                AS5600 Encoder (Optional)
                ┌─────────────────┐
                │                 │
      SDA <─────┤ SDA        VCC ├────< 3.3V
      SCL <─────┤ SCL        GND ├────< GND
                │                 │
                │  [Magnet Above] │
                └─────────────────┘

                OLED Display 0.42"
                ┌─────────────────┐
                │                 │
      SDA <─────┤ SDA        VCC ├────< 3.3V
      SCL <─────┤ SCL        GND ├────< GND
                │                 │
                └─────────────────┘

                NEMA 17 Stepper Motor
                ┌─────────────────┐
                │                 │
       A+ <─────┤ Black           │
       A- <─────┤ Green           │
       B+ <─────┤ Red             │
       B- <─────┤ Blue            │
                │                 │
                └─────────────────┘
```

## Important Notes

### TMC2130 Configuration Jumpers

1. **SPI Mode**: Ensure the TMC2130 board is configured for SPI mode (not standalone)
2. **Sense Resistors**: Most TMC2130 boards have 0.11Ω sense resistors
3. **CFG Pins**: Leave CFG4 and CFG5 open or pulled to GND for SPI mode

### Voltage Requirements

- **Logic Voltage (VIO)**: 3.3V from ESP32-C3
- **Motor Voltage (VM)**: 12-24V DC (separate power supply)
- **Motor Current**: Configurable via software (100-1500mA RMS)

### Key Features of TMC2130

1. **SPI Communication**: Full bidirectional control and diagnostics
2. **StallGuard2**: Sensorless homing and stall detection
3. **StealthChop**: Ultra-quiet motor operation
4. **SpreadCycle**: Dynamic current control for smooth movement
5. **256 Microstepping**: Extremely smooth motion

### Differences from TMC2209

| Feature | TMC2130 | TMC2209 |
|---------|---------|---------|
| Communication | SPI (4 wires) | UART (2 wires) |
| Speed | Faster | Slower |
| Diagnostics | Full bidirectional | Full bidirectional |
| StallGuard | StallGuard2 | StallGuard4 |
| CoolStep | No | Yes |
| Price | Higher | Lower |

### Pull-up/Pull-down Resistors

- CS pin needs 10kΩ pull-up to 3.3V when not selected
- EN pin has internal pull-down, drive HIGH to enable motor
- DIAG0/DIAG1 pins can be used for stall detection (optional)

### Heat Dissipation

- Add heatsink for currents above 1A RMS
- Ensure adequate ventilation in enclosure
- TMC2130 has thermal protection but will reduce current if overheating

## Sample Arduino Code for Testing

```cpp
#include <TMCStepper.h>
#include <SPI.h>

// Define pins
#define STEP_PIN 2
#define DIR_PIN 3
#define EN_PIN 4
#define CS_PIN 10

// TMC2130 configuration
#define R_SENSE 0.11f

TMC2130Stepper driver(CS_PIN, R_SENSE);

void setup() {
  // Configure pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // Disable initially

  // Start SPI
  SPI.begin();

  // Initialize TMC2130
  driver.begin();
  driver.toff(5);              // Enable driver
  driver.rms_current(800);     // Set motor current to 800mA
  driver.microsteps(16);        // Set to 16 microsteps
  driver.en_pwm_mode(true);     // Enable StealthChop
  driver.pwm_autoscale(true);   // Automatic current scaling
  driver.sgt(8);                // StallGuard threshold

  // Enable motor
  digitalWrite(EN_PIN, LOW);    // Active low
}

void loop() {
  // Simple step test
  digitalWrite(DIR_PIN, HIGH);
  for(int i = 0; i < 200; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
  }
  delay(1000);
}
```

## Sensorless Homing with StallGuard2

The TMC2130 supports sensorless homing using StallGuard2:

```cpp
bool performSensorlessHoming() {
  driver.sgt(8);  // Set sensitive threshold
  driver.sfilt(1); // Enable filter

  // Move slowly towards home
  digitalWrite(DIR_PIN, LOW);
  while(true) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);

    // Check for stall (sg_result == 0 means stall)
    if(driver.sg_result() == 0) {
      Serial.println("Home position found!");
      return true;
    }
  }
}
```

## Troubleshooting

1. **Motor not moving**: Check EN pin is LOW and SPI communication is working
2. **SPI communication fails**: Verify CS pin and SPI connections
3. **Motor vibrating**: Incorrect coil connections, swap A+/A- or B+/B-
4. **Overheating**: Reduce current or add better cooling
5. **Position drift**: Increase current or reduce acceleration
6. **StallGuard not detecting**: Adjust sgt threshold (lower = more sensitive)

## Configuration Commands (Serial)

When TMC2130 is selected, these commands are available:

- `#TMC_MS[value]` - Set microsteps (1, 2, 4, 8, 16, 32, 64, 128, 256)
- `#TMC_CUR[value]` - Set motor current in mA (100-1500)
- `#TMC_STATUS` - Get TMC2130 status and diagnostics
- `#TMC_SC[0/1]` - Enable/disable StealthChop
- `#TMC_SG[0/1]` - Enable/disable StallGuard
- `#TMC_SGT[value]` - Set StallGuard threshold (-64 to 63)
- `#TMC_HOME` - Perform sensorless homing
- `#TMC_RESET` - Reset TMC2130 to defaults