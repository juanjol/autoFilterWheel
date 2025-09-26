# ESP32-C3 Filter Wheel Controller

![Filter Wheel Controller](images/hero-image.png){ align=right width="300" }

A professional, open-source astronomy filter wheel controller built on the ESP32-C3 microcontroller. Features integrated OLED display, stepper motor control, magnetic encoder feedback, and full ASCOM compatibility for seamless integration with popular astronomy software.

## ✨ Key Features

### 🔄 **Advanced Motor Control**
- **28BYJ-48 stepper motor** with precise positioning
- **Configurable speed, acceleration, and direction** settings
- **Backlash compensation** for improved accuracy
- **Automatic power management** to reduce heat and power consumption

### 📟 **Integrated Display**
- **0.42" OLED display** (72x40 visible area)
- Real-time position and status information
- Filter name display with customizable labels
- Movement progress indication

### 🎯 **Precision Positioning**
- **AS5600 magnetic encoder** for position feedback (optional)
- **Revolution calibration** for exact step counting
- **Backlash calibration** for mechanical compensation
- **Multiple positioning modes** (unidirectional/bidirectional)

### 🔧 **Full Configuration Control**
- **3-8 filter positions** configurable via commands
- **Custom filter names** up to 15 characters each
- **Dynamic motor parameters** without firmware recompilation
- **Persistent EEPROM storage** for all settings

### 🌐 **ASCOM Compatible**
- **Complete ASCOM driver integration**
- **Serial protocol** compatible with astronomy software
- **NINA, PHD2, and other software** support
- **115200 baud serial communication**

## 🚀 Quick Start

1. **[Hardware Setup](getting-started/hardware.md)** - Gather required components
2. **[Assembly Guide](getting-started/assembly.md)** - Build your filter wheel
3. **[Wiring Diagram](getting-started/wiring.md)** - Connect all components
4. **[Firmware Installation](getting-started/firmware.md)** - Flash the ESP32-C3
5. **[First Setup](user-guide/setup.md)** - Configure your filter wheel

## 🎬 Demo Video

<div class="video-wrapper">
  <iframe width="560" height="315" src="https://www.youtube.com/embed/your-demo-video" frameborder="0" allowfullscreen></iframe>
</div>

## 📋 Specifications

| Feature | Specification |
|---------|---------------|
| **Microcontroller** | ESP32-C3 (160MHz, WiFi, BLE) |
| **Motor** | 28BYJ-48 stepper (2048 steps/revolution) |
| **Display** | 0.42" OLED SSD1306 (128x64, I2C) |
| **Encoder** | AS5600 magnetic (12-bit resolution) |
| **Filters** | 3-8 positions (configurable) |
| **Communication** | USB Serial (115200 baud) |
| **Power** | 5V USB or external 5V/12V |
| **Dimensions** | Compact desktop form factor |

## 🛠️ Supported Commands

Over **50 serial commands** for complete control:

=== "Basic Operations"
    ```bash
    #GP        # Get current position
    #MP3       # Move to position 3
    #STATUS    # Get system status
    ```

=== "Configuration"
    ```bash
    #FC5       # Set 5 filters
    #SN1:Luminance  # Set filter name
    #MS1000    # Set motor speed
    ```

=== "Calibration"
    ```bash
    #REVCAL    # Calibrate revolution
    #BLCAL     # Calibrate backlash
    #CAL       # Set home position
    ```

[View Complete Command Reference →](commands/overview.md){ .md-button .md-button--primary }

## 🏗️ Architecture Overview

```mermaid
graph TB
    A[ESP32-C3 Controller] --> B[ULN2003 Driver]
    B --> C[28BYJ-48 Stepper Motor]
    C --> D[Filter Wheel Assembly]
    A --> E[AS5600 Encoder]
    E --> D
    A --> F[0.42" OLED Display]
    A --> G[USB Serial Interface]
    G --> H[ASCOM Driver]
    H --> I[Astronomy Software]
    I --> J[NINA/PHD2/etc.]
```

## 🌟 Why Choose This Controller?

### **Professional Grade**
Built with astronomy applications in mind, featuring robust error handling, comprehensive calibration, and professional serial protocol implementation.

### **Open Source**
Complete source code, documentation, and hardware designs available. MIT license allows commercial use and modifications.

### **Extensible**
Modular design allows easy addition of features like WiFi control, temperature sensors, or additional positioning modes.

### **Community Driven**
Active development with community contributions, regular updates, and responsive support.

## 📦 What's Included

- **Complete firmware source code** (PlatformIO project)
- **Comprehensive documentation** (this site!)
- **ASCOM driver** for Windows integration
- **3D printable enclosure** designs (STL files)
- **Wiring diagrams** and assembly instructions
- **Example configurations** for common setups

## 🤝 Community & Support

- **GitHub Repository**: [Source code and issues](https://github.com/yourusername/esp32-filter-wheel)
- **Discord Community**: [Real-time chat and support](#)
- **Forum**: [CloudyNights thread](#)
- **Email**: [project@example.com](mailto:project@example.com)

## 📈 Project Status

![GitHub Stars](https://img.shields.io/github/stars/yourusername/esp32-filter-wheel?style=social)
![GitHub Forks](https://img.shields.io/github/forks/yourusername/esp32-filter-wheel?style=social)
![GitHub Issues](https://img.shields.io/github/issues/yourusername/esp32-filter-wheel)
![GitHub License](https://img.shields.io/github/license/yourusername/esp32-filter-wheel)

- ✅ **Stable Release** - v1.0.0 available
- 🔄 **Active Development** - Regular updates and improvements
- 🧪 **Tested** - Extensive field testing by astronomy community
- 📚 **Well Documented** - Comprehensive guides and references

---

*Ready to build your own professional filter wheel controller? [Get started now!](getting-started/overview.md)*