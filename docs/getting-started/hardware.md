# Hardware Requirements

This page details all the components needed to build your ESP32-C3 Filter Wheel Controller. Components are organized by priority, with alternatives and sourcing information provided.

## 🔧 Core Components (Required)

These components are essential for basic functionality:

### ESP32-C3 Development Board
**The brain of your filter wheel controller**

| Specification | Requirement | Recommended |
|---------------|-------------|-------------|
| **Chip** | ESP32-C3 | ESP32-C3-DevKitM-1 |
| **Flash** | Minimum 4MB | 4MB or higher |
| **RAM** | 400KB (built-in) | Standard |
| **USB** | USB-C preferred | Native USB support |
| **GPIO** | Minimum 8 pins | 13+ available pins |

=== "Recommended Boards"
    - **Espressif ESP32-C3-DevKitM-1** - Official board, excellent support
    - **Seeed XIAO ESP32C3** - Ultra-compact form factor
    - **M5Stack STAMP-C3** - Integrated antenna, robust design

=== "Where to Buy"
    - **AliExpress**: $3-6 (longer shipping)
    - **Amazon**: $8-12 (fast shipping)
    - **Mouser/DigiKey**: $6-10 (authentic, reliable)
    - **Adafruit**: $8-10 (excellent support)

!!! tip "Board Selection"
    Any ESP32-C3 board will work, but boards with USB-C and good pin labeling make development easier.

---

### 28BYJ-48 Stepper Motor + ULN2003 Driver
**Provides precise rotational movement**

| Specification | Value | Notes |
|---------------|-------|-------|
| **Type** | 28BYJ-48 Unipolar Stepper | Industry standard |
| **Steps/Revolution** | 2048 (with gearbox) | 64:1 internal gearing |
| **Voltage** | 5V or 12V versions | 5V recommended |
| **Torque** | ~300 g⋅cm at 5V | Sufficient for filter wheels |
| **Driver** | ULN2003 Darlington Array | Usually included with motor |

=== "Kit Contents"
    Most kits include:
    - 28BYJ-48 stepper motor
    - ULN2003 driver board
    - 5-pin connector cable
    - Mounting screws

=== "Sourcing"
    - **AliExpress**: $2-4 per kit
    - **Amazon**: $8-12 for pack of 5
    - **Local electronics stores**: $5-8 each

!!! warning "Voltage Compatibility"
    Ensure you get the 5V version of the motor, or use external 12V supply for 12V version.

---

### 0.42" OLED Display (SSD1306)
**Provides real-time status display**

| Specification | Requirement | Notes |
|---------------|-------------|-------|
| **Size** | 0.42 inch diagonal | 72×40 visible pixels |
| **Controller** | SSD1306 or compatible | I2C interface required |
| **Voltage** | 3.3V operation | ESP32-C3 compatible |
| **Interface** | I2C (4 pins: VCC, GND, SCL, SDA) | Standard connection |

=== "Display Features"
    - **Resolution**: 128×64 pixels (72×40 visible)
    - **Colors**: Monochrome (white on black)
    - **Viewing angle**: ~160 degrees
    - **Interface**: I2C (address 0x3C)

=== "Alternative Sizes"
    - **0.96"** - Larger, easier to read (requires code changes)
    - **1.3"** - Even larger option (requires code changes)
    - **0.42"** - Exact fit for compact designs (recommended)

**Cost**: $3-8 depending on supplier and quantity

---

## 🎯 Precision Components (Highly Recommended)

### AS5600 Magnetic Rotary Encoder
**Enables precise position feedback and calibration**

| Specification | Value | Benefits |
|---------------|-------|----------|
| **Resolution** | 12-bit (4096 positions/rev) | Sub-degree accuracy |
| **Interface** | I2C (address 0x36) | Easy integration |
| **Magnet** | 6mm diametrically magnetized | Usually included |
| **Distance** | 0.5-3mm from magnet | Flexible mounting |

=== "Why Include This?"
    ✅ **Backlash calibration** - Automatic mechanical compensation
    ✅ **Position verification** - Confirms accurate positioning
    ✅ **Missed step detection** - Recovers from motor errors
    ✅ **Sub-step accuracy** - Better than stepper resolution

=== "Kit Contents"
    - AS5600 breakout board
    - 6mm diametric magnet
    - Pin headers (may need soldering)

**Cost**: $3-10 depending on supplier

!!! note "Optional but Recommended"
    While not strictly required, the encoder significantly improves accuracy and enables advanced calibration features.

---

## 🔌 Connection Components

### Breadboard or Perfboard
**For prototyping and permanent connections**

=== "Prototyping Phase"
    - **Half-size breadboard** - Good for initial testing
    - **Full-size breadboard** - More space for components
    - **Breadboard jumper wires** - Various male-to-male, female-to-male

=== "Permanent Build"
    - **Perfboard/stripboard** - For soldered connections
    - **Custom PCB** - Advanced option, not necessary

**Cost**: $3-10 for breadboard setup, $5-15 for perfboard materials

---

### Connecting Wires
**Various wire types for different connections**

| Connection Type | Wire Type | Length | Quantity |
|----------------|-----------|--------|----------|
| **Breadboard** | Male-to-Male jumpers | 10-20cm | 20+ pieces |
| **Board-to-Board** | Female-to-Male jumpers | 10-20cm | 10+ pieces |
| **Power** | 22-24 AWG stranded | 50cm | 2 colors (red/black) |
| **Stepper Motor** | 5-pin connector | Included with motor | 1 piece |

**Cost**: $5-10 for assorted wire kit

---

## 🏠 Enclosure Options

### 3D Printed Enclosure
**Custom-designed housing for your controller**

=== "Design Files Provided"
    - **STL files** for direct 3D printing
    - **OpenSCAD source** for customization
    - **Multiple variants** for different configurations

=== "Printing Requirements"
    - **Material**: PLA or PETG
    - **Layer height**: 0.2mm
    - **Support**: Minimal required
    - **Print time**: 2-4 hours depending on size

**Cost**: $2-5 in filament (if you have access to 3D printer)

---

### Alternative Enclosures

=== "Project Box"
    - **Plastic electronics enclosure** 100×80×40mm
    - **Cost**: $8-15
    - **Pros**: Professional appearance, readily available
    - **Cons**: Requires drilling for connectors

=== "Laser Cut Acrylic"
    - **Clear or colored acrylic** panels
    - **Cost**: $10-20 (if you have laser cutter access)
    - **Pros**: Clean appearance, customizable
    - **Cons**: Requires access to laser cutter

=== "DIY Solutions"
    - **Cardboard prototype** - Free, temporary solution
    - **Wooden box** - Natural materials, customizable
    - **Repurposed enclosure** - Cost-effective option

---

## ⚡ Power Options

### USB Power (Recommended for Most Users)
**Simple and convenient power solution**

- **Source**: USB-C cable from ESP32-C3 to computer/USB hub
- **Voltage**: 5V regulated by ESP32-C3 board
- **Current**: ~500mA maximum (sufficient for most operations)
- **Pros**: Simple, no additional components needed
- **Cons**: May not provide enough current for high-torque operations

---

### External 5V Supply (High Performance)
**For demanding applications requiring more torque**

| Specification | Requirement |
|---------------|-------------|
| **Voltage** | 5V ±5% |
| **Current** | Minimum 1A, 2A recommended |
| **Connector** | Barrel jack or screw terminals |
| **Regulation** | Switching supply preferred |

**When to Use**:
- Larger filter wheels requiring more torque
- Continuous operation scenarios
- Maximum movement speed requirements

---

### External 12V Supply (Maximum Performance)
**For the highest torque and speed requirements**

!!! warning "Advanced Configuration"
    Requires 12V version of 28BYJ-48 motor and voltage level conversion for ESP32-C3 (which operates at 3.3V).

---

## 🛠️ Tools Required

### Basic Tools (Essential)
- **Soldering iron** (temperature controlled preferred)
- **Solder** (60/40 or lead-free)
- **Wire strippers** for various wire gauges
- **Small screwdrivers** (Phillips and flathead)
- **Multimeter** for testing connections

### Helpful Tools
- **Breadboard** for prototyping
- **Desoldering braid** for corrections
- **Flux** for easier soldering
- **Heat shrink tubing** for insulation
- **Cable ties** for wire management

### Advanced Tools (Optional)
- **3D printer** for custom enclosure
- **Oscilloscope** for advanced debugging
- **Logic analyzer** for protocol debugging

---

## 💰 Complete Bill of Materials

### Budget Build (~$25-35)
| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| ESP32-C3 Board | 1 | $6 | $6 |
| 28BYJ-48 + ULN2003 | 1 | $4 | $4 |
| 0.42" OLED Display | 1 | $5 | $5 |
| Breadboard & Wires | 1 set | $8 | $8 |
| Basic Enclosure | 1 | $5 | $5 |
| **Budget Total** | | | **~$28** |

### Recommended Build (~$40-55)
| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| Previous items | | | $28 |
| AS5600 Encoder | 1 | $8 | $8 |
| External 5V Supply | 1 | $12 | $12 |
| Better Enclosure | 1 | $10 | $10 |
| **Recommended Total** | | | **~$58** |

### Premium Build (~$70-90)
| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| Previous items | | | $58 |
| Quality connectors | 1 set | $8 | $8 |
| Professional enclosure | 1 | $15 | $15 |
| Extra components | | $10 | $10 |
| **Premium Total** | | | **~$91** |

---

## 🛒 Sourcing Strategy

### Best Practices
1. **Start with basics** - Get core components first, add encoder later
2. **Buy kits when available** - Often cheaper than individual components
3. **Consider shipping** - Consolidate orders to save on shipping costs
4. **Plan for spares** - Get a few extra wires and connectors

### Recommended Suppliers

=== "For Speed"
    - **Amazon** - Fast shipping, higher prices
    - **Adafruit** - Quality components, great support
    - **SparkFun** - Reliable, good documentation

=== "For Value"
    - **AliExpress** - Lowest prices, slower shipping
    - **Banggood** - Good compromise of price/speed
    - **LCSC** - Electronic components specialist

=== "For Reliability"
    - **Mouser** - Authentic components, premium pricing
    - **DigiKey** - Extensive catalog, fast shipping
    - **Arrow** - Professional supplier, bulk options

---

**Next Step:** Once you have your components, proceed to the **[Assembly Guide →](assembly.md)**