# DeviceLib - Arduino IoT Library

[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](release-notes/)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP8266-lightgrey.svg)](platformio.ini)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A powerful and lightweight Arduino library that transforms traditional polling-based I/O into modern event-driven programming for ESP32/ESP8266. Features advanced button handling, smart output control, and thread-safe callback execution with automatic memory management.

## ✨ Features

- 🎯 **Event-Driven Programming** - Respond to button events through callback functions instead of polling loops. Support for single clicks, double clicks, long presses, and custom hold timing
- 🔄 **Smart Debouncing** - Hardware-level debouncing eliminates false triggers and button noise with configurable timing thresholds
- ⏱️ **Advanced Timing Control** - Sophisticated timing system for multi-click detection, variable hold durations, and idle state management
- 🧠 **Memory Safe Architecture** - RAII implementation with automatic timer cleanup prevents memory leaks and resource exhaustion
- 🌐 **Cross-Platform Compatibility** - Unified API that works seamlessly across ESP32 and ESP8266 with platform-optimized timer implementations
- 🔌 **I2C GPIO Expansion** - Native support for PCF8574/PCF8575 expanders to increase available I/O pins via I2C interface
- ⚡ **Thread-Safe Callbacks** - Advanced scheduling system ensures safe execution in both interrupt and main loop contexts
- 🔧 **Flexible Configuration** - Extensive customization options with intelligent defaults for rapid prototyping

## 🚀 Quick Start

### Installation

**PlatformIO:**

```ini
[env:your_board]
lib_deps = 
    nht173/DeviceLib@^1.2.0
    # For PCF8574/8575 support (optional)
    https://github.com/nht173/PCF8574_library.git
```

**Arduino IDE:**

1. Download DeviceLib from [releases](https://github.com/nht173/DeviceLib/releases)
2. Install via Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. For PCF8574/8575 support: Download and install the [custom PCF8574 library](https://github.com/nht173/PCF8574_library)

### Basic Usage

```cpp
#include "GenericButton.h"
#include "GenericOutput.h"

GenericButton button(10, INPUT_PULLUP, LOW);
GenericOutput led(LED_BUILTIN, LOW, START_UP_LAST_STATE);

void setup() {
    Serial.begin(115200);
  
    led.begin();
  
    // Single click
    button.onClick([]() {
        led.toggle();
        Serial.println("Button clicked!");
    });
  
    // Long press (3 seconds)
    button.onLongClick([]() {
        Serial.println("Long press detected!");
    });
  
    // Double click
    button.onDoubleClick([]() {
        Serial.println("Double click!");
    });
}

void loop() {
#ifdef ESP32
    GPIO_Scheduler.run();
#endif
}
```

## 📖 Documentation

- 📋 **[Release Notes](release-notes/)** - User-friendly release summaries
- 🔧 **[Changelog](CHANGELOG.md)** - Technical changes for developers
- 📁 **[Examples](examples/)** - Complete working examples
- 📚 **[API Reference](API.md)** - Complete API documentation

## 🚀 Quick Examples

### Basic Button Control

```cpp
#include "GenericButton.h"
#include "GenericOutput.h"

GenericButton button(10, INPUT_PULLUP, LOW);
GenericOutput led(LED_BUILTIN, LOW, START_UP_LAST_STATE);

void setup() {
    Serial.begin(115200);
    led.begin();
  
    // Single click
    button.onClick([]() {
        led.toggle();
        Serial.println("LED toggled!");
    });
  
    // Double click
    button.onDoubleClick([]() {
        led.on(0, 1000); // On for 1 second
        Serial.println("LED blink!");
    });
  
    // Long press  
    button.onLongClick([]() {
        Serial.println("Long press detected!");
    });
}

void loop() {
#ifdef ESP32
    GPIO_Scheduler.run();
#endif
}

```

## 🔌 PCF8574/8575 Support

Expand your I/O using I2C expanders. See [API Documentation](API.md#pcf8574-8575-support) for complete details.

> **⚠️ Important:** Use the custom PCF8574 library for optimal compatibility with DeviceLib. The standard library may have compatibility issues.

**Recommended PCF8574 Library:**
```ini
# PlatformIO
lib_deps = 
    nht173/DeviceLib@^1.2.0
    https://github.com/nht173/PCF8574_library.git  # Custom fork
```

```cpp
#include <PCF8574_library.h>
#include "GenericButton.h"

PCF8574 expander(0x20);
GenericButton expanderBtn(expander, 0, INPUT, LOW);
GenericOutput expanderLed(expander, 1, LOW);
```

## 🔧 Advanced Features

### Memory Management (v1.2.0+)

Automatic cleanup prevents memory leaks with RAII implementation:

```cpp
void dynamicButtons() {
    for (int i = 0; i < 10; i++) {
        GenericButton button(i + 10);
        button.onClick([]() { Serial.println("Clicked!"); });
        // Timer resources automatically cleaned up when scope ends
    }
}
```

### Dynamic Callback Management

```cpp
// Mode switching example
void switchToMode1() {
    button.clearCallbacks();
    button.onClick([]() { Serial.println("Mode 1 action"); });
}

void switchToMode2() {
    button.clearCallbacks();  
    button.onClick([]() { Serial.println("Mode 2 action"); });
}
```

For complete API documentation including all classes, methods, and advanced features, see **[API.md](API.md)**.

## ⚡ Performance

- **Memory**: ~200 bytes per button/output object
- **CPU**: Minimal overhead, interrupt-driven
- **Timing**: Microsecond precision on ESP32
- **Reliability**: Production-tested in IoT deployments

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built for the Arduino/ESP32/ESP8266 community
- Inspired by real-world IoT project requirements
- Thanks to all contributors and testers
