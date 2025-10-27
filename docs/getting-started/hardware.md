# Hardware Requirements

Complete list of components needed to build the ESP32-C3 Filter Wheel Controller.

## Core Components

### ESP32-C3 Development Board

| Specification | Requirement |
|---------------|-------------|
| Chip | ESP32-C3 |
| Flash | 4MB minimum |
| RAM | 400KB (built-in) |
| USB | USB-C preferred |
| GPIO | Minimum 8 pins |

**Recommended boards:**
- Espressif ESP32-C3-DevKitM-1
- Seeed XIAO ESP32C3
- M5Stack STAMP-C3

**Cost:** $3-12 depending on supplier

---

### 28BYJ-48 Stepper Motor + ULN2003 Driver

| Specification | Value |
|---------------|-------|
| Type | 28BYJ-48 Unipolar Stepper |
| Steps/Revolution | 2048 (with gearbox) |
| Voltage | 5V recommended |
| Torque | ~300 g⋅cm at 5V |
| Driver | ULN2003 Darlington Array |

**Kit includes:**
- 28BYJ-48 stepper motor
- ULN2003 driver board
- 5-pin connector cable
- Mounting screws

**Cost:** $2-12 per kit

---

### 0.42" OLED Display (SSD1306)

| Specification | Value |
|---------------|-------|
| Size | 0.42 inch diagonal |
| Resolution | 128×64 pixels (72×40 visible) |
| Controller | SSD1306 or compatible |
| Interface | I2C (4 pins: VCC, GND, SCL, SDA) |
| Voltage | 3.3V |
| I2C Address | 0x3C |

**Cost:** $3-8

---

## Precision Components

### AS5600 Magnetic Rotary Encoder

| Specification | Value |
|---------------|-------|
| Resolution | 12-bit (4096 positions/rev) |
| Accuracy | 0.088° per count |
| Interface | I2C (address 0x36) |
| Magnet | 6mm diametrically magnetized |
| Distance | 0.5-3mm from magnet |

**Required for PID control.** Provides:
- Sub-degree positioning accuracy
- Automatic error correction
- Missed step detection
- Position verification

**Kit includes:**
- AS5600 breakout board
- 6mm diametric magnet
- Pin headers

**Cost:** $3-10

---

## Connection Components

### Breadboard / Perfboard

**Prototyping:**
- Half-size or full-size breadboard
- Breadboard jumper wires (male-to-male, female-to-male)

**Permanent Build:**
- Perfboard/stripboard for soldering
- 22-24 AWG stranded wire

**Cost:** $3-15

---

### Connecting Wires

| Connection Type | Wire Type | Quantity |
|----------------|-----------|----------|
| Breadboard | Male-to-Male jumpers, 10-20cm | 20+ pieces |
| Board-to-Board | Female-to-Male jumpers, 10-20cm | 10+ pieces |
| Power | 22-24 AWG stranded | 50cm (red/black) |
| Stepper Motor | 5-pin connector | Included with motor |

**Cost:** $5-10

---

## Power Supply

### USB Power (Recommended)
- **Source:** USB-C cable from ESP32-C3
- **Voltage:** 5V
- **Current:** ~500mA maximum
- **Sufficient for most applications**

### External 5V Supply (Optional)
- **Voltage:** 5V ±5%
- **Current:** 1-2A
- **Use for:** Higher torque requirements or continuous operation

---

## Tools Required

### Basic Tools
- Soldering iron (temperature controlled)
- Solder (60/40 or lead-free)
- Wire strippers
- Small screwdrivers (Phillips and flathead)
- Multimeter

### Helpful Tools
- Desoldering braid
- Flux
- Heat shrink tubing
- Cable ties

---

## Bill of Materials

### Budget Build (~$25-35)
| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| ESP32-C3 Board | 1 | $6 | $6 |
| 28BYJ-48 + ULN2003 | 1 | $4 | $4 |
| 0.42" OLED Display | 1 | $5 | $5 |
| Breadboard & Wires | 1 set | $8 | $8 |
| **Total** | | | **~$23** |

### Recommended Build (~$35-50)
| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| Previous items | | | $23 |
| AS5600 Encoder | 1 | $8 | $8 |
| External 5V Supply | 1 | $12 | $12 |
| **Total** | | | **~$43** |

---

## Suppliers

**Fast Shipping:**
- Amazon, Adafruit, SparkFun

**Best Value:**
- AliExpress, Banggood, LCSC

**Authentic Components:**
- Mouser, DigiKey, Arrow

---

Next: [Assembly Guide](assembly-uln2003.md)
