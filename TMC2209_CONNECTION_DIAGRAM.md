# TMC2209 Connection Diagram for ESP32-C3 OLED Filter Wheel Controller

## Pin Connections

### ESP32-C3 OLED Board to TMC2209

```
ESP32-C3 OLED          TMC2209 Driver
--------------         --------------
GPIO 2  (STEP)    -->  STEP
GPIO 3  (DIR)     -->  DIR
GPIO 4  (EN)      -->  EN (Enable)
GPIO 7  (RX)      -->  TX (PDN/UART)
GPIO 10 (TX)      -->  RX (jumper to PDN)

GPIO 5  (SDA)     -->  I2C OLED/AS5600
GPIO 6  (SCL)     -->  I2C OLED/AS5600

3.3V              -->  VIO (Logic voltage)
GND               -->  GND (Logic ground)
```

### Power Connections

```
Motor Power Supply     TMC2209 Driver
-----------------     --------------
12-24V DC (+)     -->  VM (Motor voltage)
GND (-)           -->  GND (Motor ground)

TMC2209           Stepper Motor (NEMA 17)
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
                │  GPIO2 ─────────┼────> STEP (TMC2209)
                │  GPIO3 ─────────┼────> DIR  (TMC2209)
                │  GPIO4 ─────────┼────> EN   (TMC2209)
                │  GPIO7 ─────────┼────> TX/PDN (TMC2209)
                │  GPIO10 ────────┼────> RX   (TMC2209)
                │                 │
                │  GPIO5 (SDA) ───┼────> SDA (OLED & AS5600)
                │  GPIO6 (SCL) ───┼────> SCL (OLED & AS5600)
                │                 │
                │  3.3V ──────────┼────> VIO (TMC2209)
                │  GND ───────────┼────> GND (TMC2209)
                │                 │
                │  USB-C Power    │
                └─────────────────┘

                TMC2209 SilentStepStick
                ┌─────────────────┐
                │                 │
     STEP <─────┤ STEP       VM  ├────< 12-24V DC
      DIR <─────┤ DIR        GND ├────< Power GND
       EN <─────┤ EN         A1  ├────> Motor A+
  TX/PDN <─────┤ PDN/TX     A2  ├────> Motor A-
       RX <─────┤ RX         B1  ├────> Motor B+
      VIO <─────┤ VIO        B2  ├────> Motor B-
      GND <─────┤ GND            │
                │                 │
                │ [Jumper Config] │
                │ UART Mode: Set  │
                │ Addr: 00        │
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

### TMC2209 Configuration Jumpers

1. **UART Mode**: Bridge the jumper on the TMC2209 board to enable UART communication
2. **Address Selection**: For single driver, set address to 00 (no additional jumpers needed)
3. **Sense Resistors**: Most TMC2209 boards have 0.11Ω sense resistors

### Voltage Requirements

- **Logic Voltage (VIO)**: 3.3V from ESP32-C3
- **Motor Voltage (VM)**: 12-24V DC (separate power supply)
- **Motor Current**: Configurable via software (100-1500mA RMS)

### TMC2208 Compatibility

The TMC2208 uses the same pin configuration but:
- TMC2208 is **write-only** UART (cannot read status)
- Configuration is one-way only
- Use same connections but expect limited feedback

### Pull-up Resistors

- UART pins may need 1-10kΩ pull-up resistors to 3.3V if communication is unstable
- EN pin has internal pull-down, drive HIGH to enable motor

### Heat Dissipation

- Add heatsink for currents above 1A RMS
- Ensure adequate ventilation in enclosure
- TMC2209 has thermal protection but will reduce current if overheating

## Sample Arduino Code for Testing

```cpp
#include <TMCStepper.h>

// Define pins
#define STEP_PIN 2
#define DIR_PIN 3
#define EN_PIN 4
#define RX_PIN 7
#define TX_PIN 10

// TMC2209 configuration
#define SERIAL_PORT Serial1
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

void setup() {
  // Configure pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  // Start UART
  SERIAL_PORT.begin(115200);

  // Initialize TMC2209
  driver.begin();
  driver.toff(5);
  driver.rms_current(800);    // Set motor current to 800mA
  driver.microsteps(16);       // Set to 16 microsteps
  driver.en_spreadCycle(false); // Use StealthChop

  // Enable motor
  digitalWrite(EN_PIN, LOW);   // Active low
}
```

## Troubleshooting

1. **Motor not moving**: Check EN pin is LOW and power supply is connected
2. **UART communication fails**: Verify TX/RX connections and pull-up resistors
3. **Motor vibrating**: Incorrect coil connections, swap A+/A- or B+/B-
4. **Overheating**: Reduce current or add better cooling
5. **Position drift**: Increase current or reduce acceleration