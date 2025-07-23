# DeviceLib API Documentation

[![Version](https://img.shields.io/badge/version-1.1.1-blue.svg)](../release-notes/)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP8266-lightgrey.svg)](../platformio.ini)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

Complete API reference for DeviceLib - Arduino IoT Library for ESP32/ESP8266 with event-driven programming support.

## 📋 Table of Contents

- [GenericButton](#genericbutton) - Event-driven button handling
- [GenericOutput](#genericoutput) - Smart output control
- [GenericInput](#genericinput) - Base input handling with debouncing
- [GenericOutputBase](#genericoutputbase) - Base output functionality
- [Callback Scheduling System](#callback-scheduling-system) - Thread-safe callback execution
- [DeviceLibTypes](#devicelibtypes) - Core types and definitions
- [PCF8574/8575 Support](#pcf8574-8575-support) - I2C GPIO expander integration
- [Error Handling](#error-handling) - Debug and error management
- [Memory Management](#memory-management) - RAII and automatic cleanup
- [Platform Differences](#platform-differences) - ESP32 vs ESP8266 specifics

---

## GenericButton

Event-driven button handling with multiple callback types, debouncing, and advanced timing features.

### 🏗️ Constructors

```cpp
// Basic GPIO constructor
GenericButton(uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

// PCF8574/8575 constructor (if USE_PCF is defined)
GenericButton(PCF_TYPE &pcf, uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

// Default constructor
GenericButton() = default;
```

**Parameters:**
- `pin` - GPIO pin number or PCF pin number (0-7 for PCF8574, 0-15 for PCF8575)
- `mode` - Pin mode: `INPUT`, `INPUT_PULLUP`, `INPUT_PULLDOWN`
- `activeState` - Active logic level: `LOW` (default) or `HIGH`
- `debounceTime` - Debounce time in milliseconds (default: 50ms)

**Example:**
```cpp
// Standard GPIO button
GenericButton button1(10, INPUT_PULLUP, LOW, 50);

// PCF8574 button
PCF8574 pcf(0x20);
GenericButton button2(pcf, 0, INPUT, LOW, 30);
```

### 🔄 State Management

#### `generic_button_state_t getState()`
Returns current button state.

**Return Values:**
- `BUTTON_STATE_IDLE` - Button is not pressed, no activity
- `BUTTON_STATE_PRESSED` - Button is currently pressed
- `BUTTON_STATE_RELEASED` - Button was just released

#### `String getStateString()`
Returns human-readable state string.

**Return Values:** `"idle"`, `"pressed"`, `"released"`

### ⚙️ Configuration Methods

#### `void setClickWaitTime(uint32_t time)`
Set wait time for multi-click detection (default: 300ms).

#### `uint32_t getClickWaitTime()`
Get current multi-click wait time.

#### `void setHoldTime(uint32_t time)`
Set threshold for long click detection (default: 3000ms).

#### `uint32_t getHoldTime()`
Get current long click threshold.

#### `void setIdleTime(uint32_t time)`
Set time to wait before returning to idle state (default: 500ms).

#### `uint32_t getIdleTime()`
Get current idle detection time.

**Example:**
```cpp
button.setClickWaitTime(250);  // Faster double-click detection
button.setHoldTime(2000);      // 2-second long press
button.setIdleTime(1000);      // 1-second idle timeout
```

### 🎯 Event Callbacks

All callback methods support the **scheduling system** for thread-safe execution. See [Callback Scheduling System](#callback-scheduling-system) for detailed information about `schedule` parameter behavior.

#### Basic Events

```cpp
void onClick(std::function<void()> cb, bool schedule = true);
void onDoubleClick(std::function<void()> cb, bool schedule = true);
void onLongClick(std::function<void()> cb, bool schedule = true);
void onPress(std::function<void()> cb, bool schedule = true);
void onRelease(std::function<void()> cb, bool schedule = true);
void onIdle(std::function<void()> cb, bool schedule = true);
```

#### Advanced Events

```cpp
// Custom click count
void onClickCount(uint8_t count, std::function<void()> cb, bool schedule = true);

// Custom hold time
void onPressHold(uint32_t hold_time, std::function<void()> cb, bool schedule = true);

// State change (any transition)
void onChange(std::function<void()> cb, bool schedule = true);
```

#### Generic Event Handler

```cpp
void onEvent(generic_button_event_t event, std::function<void()> cb, uint32_t param = 0, bool schedule = true);
```

**Event Types:**
- `BUTTON_EVENT_IDLE` - Button returns to idle
- `BUTTON_EVENT_CLICK` - Single click detected
- `BUTTON_EVENT_DOUBLE_CLICK` - Double click detected
- `BUTTON_EVENT_LONG_CLICK` - Long click detected
- `BUTTON_EVENT_CLICK_COUNT` - N clicks detected (use param for count)
- `BUTTON_EVENT_PRESS_HOLD` - Custom hold time (use param for time in ms)
- `BUTTON_EVENT_PRESSED` - Button pressed down
- `BUTTON_EVENT_RELEASED` - Button released
- `BUTTON_EVENT_STATE_CHANGE` - Any state change

**Parameters:**
- `cb` - Callback function to execute
- `schedule` - **Execution mode:** `true` = scheduled (safe, default), `false` = immediate (ISR). See [Callback Scheduling](#callback-scheduling-system) for details.
- `param` - Additional parameter for `CLICK_COUNT` (count) or `PRESS_HOLD` (time in ms)

### 🧹 Callback Management (v1.1.1+)

```cpp
void clearCallbacks();                                    // Remove all callbacks
void removeCallbacks(generic_button_event_t event);      // Remove specific event callbacks
```

### 📝 Complete Example

```cpp
#include "GenericButton.h"

GenericButton button(10, INPUT_PULLUP, LOW);

void setup() {
    Serial.begin(115200);
    
    // Basic events
    button.onClick([]() {
        Serial.println("Single click!");
    });
    
    button.onDoubleClick([]() {
        Serial.println("Double click!");
    });
    
    button.onLongClick([]() {
        Serial.println("Long press (3+ seconds)!");
    });
    
    // Advanced events
    button.onClickCount(3, []() {
        Serial.println("Triple click!");
    });
    
    button.onPressHold(1500, []() {
        Serial.println("1.5 second hold!");
    });
    
    // State monitoring
    button.onPress([]() {
        Serial.println("Button pressed down");
    });
    
    button.onRelease([]() {
        Serial.println("Button released");
    });
    
    button.onIdle([]() {
        Serial.println("Button idle");
    });
    
    // Configuration
    button.setClickWaitTime(250);  // Fast double-click
    button.setHoldTime(2500);      // 2.5s long press
}

void loop() {
#ifdef ESP32
    GPIO_Scheduler.run();  // Required for ESP32
#endif
}
```

---

## GenericOutput

Smart output control with auto-off functionality, timed operations, and state management.

### 🏗️ Constructors

```cpp
// Basic GPIO constructor
GenericOutput(uint8_t pin, bool activeState = LOW, startup_state_t startupState = START_UP_NONE, uint32_t duration = 0);

// PCF8574/8575 constructor (if USE_PCF is defined)
GenericOutput(PCF_TYPE &pcf, uint8_t pin, bool activeState = LOW, startup_state_t startupState = START_UP_NONE, uint32_t duration = 0);

// Default constructor
GenericOutput() = default;
```

**Parameters:**
- `pin` - GPIO pin number or PCF pin number
- `activeState` - Active logic level: `LOW` (default) or `HIGH`
- `startupState` - Power-on behavior (see Startup States below)
- `duration` - Auto-off duration in milliseconds (0 = disabled)

**Startup States:**
- `START_UP_NONE` - No automatic startup behavior
- `START_UP_OFF` - Start in OFF state
- `START_UP_ON` - Start in ON state
- `START_UP_LAST_STATE` - Restore last state before power-off (requires filesystem)

### 🔧 Initialization

#### `void begin()`
**Must be called in `setup()`** to initialize timers and apply startup state.

```cpp
void setup() {
    output.begin();  // Required!
}
```

### ⚡ Basic Control

```cpp
void on();                                    // Turn on
void on(bool force);                         // Turn on with force override
void off();                                  // Turn off
void off(bool force);                        // Turn off with force override
void toggle();                               // Toggle current state
```

**Force Parameter:**
- `false` - Respects auto-off and delay settings
- `true` - Bypasses all timing controls, immediate action

### ⏱️ Timed Operations

```cpp
// Turn on with delay and duration (one-time operation)
void on(uint32_t onDelay, uint32_t duration, bool force = true);

// Turn on for percentage of configured duration
void onPercentage(uint8_t percentage, bool force = false);
```

**Examples:**
```cpp
output.on(1000, 5000);    // Wait 1s, then turn on for 5s
output.on(0, 3000);       // Turn on immediately for 3s
output.onPercentage(50);  // Turn on for 50% of configured duration
```

### ⚙️ Configuration

#### Auto-Off Settings

```cpp
void setAutoOff(bool enabled);                           // Enable/disable auto-off
void setAutoOff(bool enabled, uint32_t duration);       // Set auto-off with duration
void setDuration(uint32_t duration);                    // Set auto-off duration only
uint32_t getDuration() const;                           // Get current duration
```

#### Power-On Delay

```cpp
void setPowerOnDelay(uint32_t delay);                   // Set delay before turning on
uint32_t getPowerOnDelay() const;                      // Get current delay
```

### 🎯 State Callbacks

All output callbacks support **scheduling system** for thread-safe execution. See [Callback Scheduling System](#callback-scheduling-system) for execution mode details.

```cpp
void onPowerOn(std::function<void()> cb, bool schedule = true);      // When turned on
void onPowerOff(std::function<void()> cb, bool schedule = true);     // When turned off
void onPowerChanged(std::function<void()> cb, bool schedule = true); // State change
void onAutoOff(std::function<void()> cb, bool schedule = true);      // Auto-off triggered
```

**Scheduling Recommendation:** Use `schedule = true` (default) for most cases. Only use `schedule = false` for time-critical responses where microsecond timing is required.

### 📝 Complete Example

```cpp
#include "GenericOutput.h"

// LED with auto-off after 5 seconds, restore last state on startup
GenericOutput led(LED_BUILTIN, LOW, START_UP_LAST_STATE, 5000);

// Relay with no auto-off
GenericOutput relay(12, HIGH, START_UP_OFF);

void setup() {
    Serial.begin(115200);
    
    // Must call begin() for all outputs
    led.begin();
    relay.begin();
    
    // Configure LED
    led.setPowerOnDelay(500);  // 500ms delay before turning on
    
    led.onPowerOn([]() {
        Serial.println("LED turned on - will auto-off in 5s");
    });
    
    led.onAutoOff([]() {
        Serial.println("LED auto-off triggered");
    });
    
    led.onPowerOff([]() {
        Serial.println("LED turned off");
    });
    
    // Configure relay
    relay.onPowerChanged([]() {
        Serial.println("Relay state changed to: " + String(relay.getState()));
    });
}

void loop() {
    // Example usage
    static unsigned long lastToggle = 0;
    if (millis() - lastToggle > 10000) {
        led.toggle();
        relay.toggle();
        lastToggle = millis();
    }
}
```

---

## GenericInput

Base class for input handling with debouncing, interrupt support, and PCF8574/8575 integration.

### 🏗️ Constructors

```cpp
// Basic GPIO constructor
GenericInput(uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

// PCF8574/8575 constructor (if USE_PCF is defined)
GenericInput(PCF_TYPE &pcf, uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

// Default constructor
GenericInput() = default;
```

### ⚙️ Configuration

```cpp
void setPin(uint8_t pin);                               // Set pin number
uint8_t getPin() const;                                 // Get pin number
void setMode(uint8_t mode);                             // Set pin mode
void setDebounceTime(uint32_t debounceTime);            // Set debounce time
uint32_t getDebounceTime() const;                       // Get debounce time
void setActiveState(bool activeState);                  // Set active state
bool getActiveState() const;                            // Get active state
```

### 🔍 State Reading

```cpp
bool getState();                                        // Get current state (true = active)
virtual String getStateString();                       // Get state as string
void setStateString(const String &active, const String &inactive); // Set state labels
```

### 🎯 Basic Callbacks

Input callbacks use **scheduling system** for safe execution. See [Callback Scheduling System](#callback-scheduling-system) for thread-safety details.

```cpp
virtual void onChange(std::function<void()> cb, bool schedule = true);    // State change
virtual void onActive(std::function<void()> cb, bool schedule = true);    // Becoming active
virtual void onInactive(std::function<void()> cb, bool schedule = true);  // Becoming inactive
```

### ⚡ Interrupt Support

```cpp
bool attachInterrupt(uint8_t mode);                     // Attach GPIO interrupt
void detachInterrupt();                                 // Detach GPIO interrupt

// PCF8574/8575 interrupt support
static bool attachInterrupt(PCF_TYPE *pcf, uint8_t boardPin);  // Attach PCF interrupt
static void processPCFIRQ();                           // Process PCF interrupts (ESP32 only)
```

**Interrupt Modes:**
- `RISING` - Trigger on rising edge
- `FALLING` - Trigger on falling edge
- `CHANGE` - Trigger on any change

### 📝 Example

```cpp
#include "GenericInput.h"

GenericInput sensor(A0, INPUT, HIGH, 100);  // Analog sensor, active HIGH, 100ms debounce

void setup() {
    Serial.begin(115200);
    
    sensor.setStateString("DETECTED", "CLEAR");
    
    sensor.onChange([]() {
        Serial.println("Sensor state: " + sensor.getStateString());
    });
    
    sensor.onActive([]() {
        Serial.println("Motion detected!");
    });
    
    sensor.onInactive([]() {
        Serial.println("Motion cleared");
    });
    
    // Attach interrupt for faster response
    sensor.attachInterrupt(CHANGE);
}

void loop() {
#ifdef ESP32
    GPIO_Scheduler.run();
#endif
}
```

---

## GenericOutputBase

Base class for output functionality with startup state management, last state persistence, and external integrations.

### 🏗️ Constructors

```cpp
// Basic GPIO constructor
GenericOutputBase(uint8_t pin, bool activeState = LOW, startup_state_t startUpState = START_UP_NONE);

// PCF8574/8575 constructor (if USE_PCF is defined)
GenericOutputBase(PCF_TYPE& pcf, uint8_t pin, bool activeState = LOW, startup_state_t startUpState = START_UP_NONE);
```

### 🔧 Core Methods

```cpp
virtual void begin();                                   // Initialize output
virtual void on();                                      // Turn on
virtual void on(bool force);                           // Turn on with force
virtual void off();                                     // Turn off
virtual void off(bool force);                          // Turn off with force
virtual void toggle();                                  // Toggle state
bool getState() const;                                  // Get current state
String getStateString() const;                         // Get state as string
```

### 🎯 Callbacks

Base output callbacks with **scheduling system** support. See [Callback Scheduling System](#callback-scheduling-system) for execution details.

```cpp
void onPowerOn(std::function<void()> cb, bool schedule = true);      // Power on event
void onPowerOff(std::function<void()> cb, bool schedule = true);     // Power off event
void onPowerChanged(std::function<void()> cb, bool schedule = true); // State change event
```

### 🔌 External Integrations

#### Firebase Realtime Database (Optional)

```cpp
void attachFirebaseRTDB(FirebaseData *fbdo, const String &path);     // Attach to Firebase
void detachFirebaseRTDB();                                           // Detach from Firebase
bool isFirebaseRTDBAttached() const;                                 // Check attachment
```

#### ESPNOW Support (Optional)

```cpp
void attachESPNOW(espnow_node_t *node, const String &key);          // Attach to ESPNOW
void detachESPNOW();                                                 // Detach from ESPNOW  
bool isESPNOWAttached() const;                                       // Check attachment
```

### 💾 State Persistence

When `START_UP_LAST_STATE` is used, the output state is automatically saved to filesystem and restored on startup.

**Supported Filesystems:**
- ESP32: SPIFFS, LittleFS, SD
- ESP8266: LittleFS, SPIFFS

---

## Callback Scheduling System

DeviceLib implements a sophisticated callback scheduling system that allows you to control when and how callbacks are executed. This is crucial for thread safety and performance optimization.

### 🎯 Schedule Parameter

All callback methods in DeviceLib accept a `schedule` parameter:

```cpp
void onClick(std::function<void()> cb, bool schedule = true);
void onPowerOn(std::function<void()> cb, bool schedule = true);
void onChange(std::function<void()> cb, bool schedule = true);
```

### 📋 Execution Modes

#### Scheduled Execution (`schedule = true`) - **Default & Recommended**

```cpp
button.onClick([]() {
    Serial.println("This runs in main loop");
    digitalWrite(LED_PIN, HIGH);  // Safe to call
}, true);  // or omit parameter (default)
```

**Characteristics:**
- ✅ **Thread-safe** - Executes in main loop context
- ✅ **ISR-safe** - Can be called from interrupt handlers
- ✅ **No blocking** - Won't block interrupt context
- ✅ **Memory safe** - Proper stack allocation
- ⚠️ **Slight delay** - Executes in next loop iteration

#### Immediate Execution (`schedule = false`) - **Advanced Use**

```cpp
button.onClick([]() {
    // This runs immediately in interrupt context!
    // Be very careful what you do here
    digitalWriteFast(LED_PIN, HIGH);  // Fast operations only
}, false);
```

**Characteristics:**
- ⚡ **Instant response** - Zero delay execution
- ⚠️ **ISR context** - Runs in interrupt handler
- ❌ **Limited functionality** - No Serial, delay(), malloc(), etc.
- ❌ **Stack limited** - Very small stack space
- ❌ **Blocking risk** - Can block other interrupts

### 🔧 Platform Implementation

#### ESP32 Implementation

```cpp
// Scheduled callbacks
void processCallback(devlib_callback_t &cb) {
    if (cb.schedule) {
        GPIO_Scheduler.addSchedule(cb.fn);  // Queued for main loop
    } else {
        cb.fn();  // Execute immediately in ISR
    }
}

// Required in main loop
void loop() {
    GPIO_Scheduler.run();  // Process scheduled callbacks
}
```

#### ESP8266 Implementation

```cpp
// Scheduled callbacks  
void processCallback(devlib_callback_t &cb) {
    if (cb.schedule) {
        schedule_function(cb.fn);  // Built-in Arduino scheduler
    } else {
        cb.fn();  // Execute immediately
    }
}

// No loop() call required - automatic
```

### 📝 Best Practices & Examples

#### ✅ **Recommended: Scheduled Callbacks**

```cpp
GenericButton button(10);
GenericOutput led(LED_BUILTIN);

void setup() {
    Serial.begin(115200);
    led.begin();
    
    // Safe: All hardware operations scheduled
    button.onClick([]() {
        Serial.println("Button clicked!");  // ✅ Safe in main loop
        led.toggle();                       // ✅ Safe GPIO operation
        delay(100);                         // ✅ Safe to use delay
    });
    
    button.onLongPress([]() {
        Serial.println("Long press - entering config mode");
        WiFi.begin("SSID", "password");    // ✅ Safe WiFi operations
        loadConfiguration();               // ✅ Safe file operations
    });
}

void loop() {
#ifdef ESP32
    GPIO_Scheduler.run();  // Required for ESP32
#endif
}
```

#### ⚡ **Advanced: Immediate Callbacks for Time-Critical Operations**

```cpp
volatile bool emergencyStop = false;

button.onPress([]() {
    // Time-critical: Immediate response required
    emergencyStop = true;           // ✅ Simple variable assignment
    digitalWriteFast(RELAY_PIN, LOW); // ✅ Fast GPIO (ESP32/ESP8266)
    // Serial.println("Emergency!");   // ❌ NOT safe in ISR
    // delay(100);                    // ❌ NOT safe in ISR
}, false);  // Immediate execution

// Handle the emergency in main loop
void loop() {
    if (emergencyStop) {
        emergencyStop = false;
        Serial.println("Emergency stop activated!");  // ✅ Safe here
        handleEmergencyProcedure();                   // ✅ Safe here
    }
    
#ifdef ESP32
    GPIO_Scheduler.run();
#endif
}
```

### 🔄 Mixed Mode Example

```cpp
GenericButton button(10);
volatile uint32_t fastCounter = 0;
uint32_t slowCounter = 0;

void setup() {
    // Fast counting (immediate)
    button.onPress([]() {
        fastCounter++;  // ✅ Fast increment in ISR
    }, false);
    
    // Logging and processing (scheduled)  
    button.onClick([]() {
        slowCounter++;
        Serial.printf("Fast: %u, Slow: %u\n", fastCounter, slowCounter);
        // Complex processing here...
    }, true);
}
```

### ⚠️ **ISR Safety Guidelines**

When using `schedule = false`, follow these rules:

#### ✅ **Safe in ISR:**
- Simple variable assignments
- `digitalWriteFast()` / `digitalReadFast()`
- Basic arithmetic operations
- Setting flags/states
- Hardware register access

#### ❌ **NOT Safe in ISR:**
- `Serial` operations (`println`, `printf`, etc.)
- `delay()`, `delayMicroseconds()` > 16μs
- `WiFi`, `Bluetooth`, `HTTP` operations
- File system operations (`SPIFFS`, `LittleFS`)
- Dynamic memory allocation (`new`, `malloc`)
- Complex library calls
- Floating-point operations (some platforms)

### 📊 Performance Comparison

| Operation | Scheduled | Immediate | Use Case |
|-----------|-----------|-----------|----------|
| Response Time | ~1-2ms | <1μs | When speed matters |
| Safety | Very High | Requires Care | Thread safety |
| Functionality | Full Arduino API | Limited | Feature requirements |
| Recommended For | General Use | Time-Critical | Default choice |

### 🐛 Common Issues & Solutions

#### Issue: "Serial output not working in callback"
```cpp
// ❌ Wrong
button.onClick([]() {
    Serial.println("This won't work!");
}, false);

// ✅ Correct  
button.onClick([]() {
    Serial.println("This works!");
}, true);  // or omit (default)
```

#### Issue: "System crashes when button pressed"
```cpp
// ❌ Wrong - complex operations in ISR
button.onClick([]() {
    WiFi.begin("SSID", "pass");  // Crashes!
}, false);

// ✅ Correct - schedule complex operations
button.onClick([]() {
    WiFi.begin("SSID", "pass");  // Safe!
});  // Default scheduled execution
```

#### Issue: "ESP32 callbacks not executing"
```cpp
void loop() {
    // ❌ Missing scheduler call
    // Callbacks won't execute!
}

void loop() {
    GPIO_Scheduler.run();  // ✅ Required for ESP32
}
```

---

## DeviceLibTypes

Core type definitions and callback structures used throughout the library.

### 🔄 Callback Structure

```cpp
struct devlib_callback_t {
    std::function<void()> fn;                           // Callback function
    bool schedule = true;                               // Schedule vs immediate execution
    
    void assign(std::function<void()> callback, bool sched = true);
    bool isValid() const;                               // Check if callback is set
    void operator()() const;                            // Execute callback
};
```

### 📋 Enumerations

#### Button Events
```cpp
enum generic_button_event_t {
    BUTTON_EVENT_IDLE,
    BUTTON_EVENT_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_CLICK,
    BUTTON_EVENT_CLICK_COUNT,
    BUTTON_EVENT_PRESS_HOLD,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_STATE_CHANGE
};
```

#### Button States
```cpp
enum generic_button_state_t {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASED
};
```

#### Startup States
```cpp
enum startup_state_t {
    START_UP_NONE,
    START_UP_OFF,
    START_UP_ON,
    START_UP_LAST_STATE
};
```

---

## PCF8574-8575 Support

Expand your I/O capabilities using I2C GPIO expanders.

> **⚠️ Library Requirement:** DeviceLib requires a custom PCF8574 library for optimal compatibility. Use the fork available at [nht173/PCF8574_library](https://github.com/nht173/PCF8574_library) instead of the standard library.

### 📦 Installation

**PlatformIO:**
```ini
lib_deps = 
    nht173/DeviceLib@^1.1.1
    https://github.com/nht173/PCF8574_library.git  # Required for PCF support
```

**Arduino IDE:**
1. Install DeviceLib
2. Download and install [custom PCF8574 library](https://github.com/nht173/PCF8574_library)

### 🔧 Setup

```cpp
#include <PCF8574_library.h>  // Use the custom library
#include "GenericButton.h"
#include "GenericOutput.h"

PCF8574 expander(0x20);    // I2C address 0x20

void setup() {
    expander.begin();
    
    // Create devices using PCF pins
    GenericButton button(expander, 0, INPUT, LOW);     // PCF pin 0
    GenericOutput led(expander, 1, LOW);               // PCF pin 1
    
    // Attach interrupt for better responsiveness (optional)
    GenericInput::attachInterrupt(&expander, 2);       // PCF INT pin connected to GPIO 2
}
```

### 🔍 Why Custom PCF8574 Library?

The custom PCF8574 library includes several improvements over standard libraries:

- **Enhanced Compatibility:** Optimized for DeviceLib's interrupt and callback system
- **Better Performance:** Improved I2C timing and error handling
- **Thread Safety:** Safe to use with DeviceLib's scheduling system
- **Debugging Support:** Additional diagnostic features for troubleshooting

**Known Issues with Standard Libraries:**
- Timing conflicts with DeviceLib's timer system
- Interrupt handling incompatibilities
- Memory management issues in some implementations

### 📊 Pin Mapping

**PCF8574:** 8 pins (0-7)
**PCF8575:** 16 pins (0-15)

### ⚡ Interrupt Handling

For ESP32, call `GenericInput::processPCFIRQ()` in your main loop for optimal interrupt processing:

```cpp
void loop() {
#ifdef ESP32
    GenericInput::processPCFIRQ();  // Process PCF interrupts
    GPIO_Scheduler.run();           // Process scheduled callbacks
#endif
}
```

---

## Error Handling

### 🐛 Debug Output

Enable debug output by defining `DEBUG` before including headers:

```cpp
#define DEBUG
#include "GenericButton.h"
```

### 📝 Error Messages

Common error patterns and solutions:

**Timer Creation Failed:**
```
[GenericOutput][Error] Failed to create timer for pin[13]: ESP_ERR_NO_MEM
```
*Solution: Reduce number of active timers or increase heap size*

**PCF Interrupt Setup Failed:**
```
[GenericInput] Failed to attach PCF interrupt on pin 2
```
*Solution: Check pin number and ensure PCF INT pin is connected*

**Null Pointer in Callback:**
```
[GenericOutput][Error] Null pointer in _onTick callback
```
*Solution: This indicates memory corruption - check object lifecycle*

---

## Memory Management

### 🧠 RAII Implementation

All DeviceLib classes implement RAII (Resource Acquisition Is Initialization) for automatic cleanup:

```cpp
void dynamicButtonExample() {
    {
        GenericButton button(10);
        button.onClick([]() { Serial.println("Clicked!"); });
        // Timer resources automatically cleaned up when button goes out of scope
    }
    // All resources are freed here
}
```

### 🔄 Destructor Behavior

- **ESP32**: Stops and deletes `esp_timer` instances
- **ESP8266**: Detaches `Ticker` instances
- **All Platforms**: Clears callback vectors

### 💡 Best Practices

1. **Always call `begin()`** for output devices
2. **Use local objects when possible** for automatic cleanup
3. **Call `clearCallbacks()`** when changing button behavior dynamically
4. **Monitor heap usage** with many dynamic objects

---

## Platform Differences

### ⏱️ Timer Implementation

| Feature | ESP32 | ESP8266 |
|---------|-------|---------|
| Timer Type | `esp_timer` | `Ticker` |
| Precision | Microsecond | Millisecond |
| Max Timers | ~50+ | ~10 |
| Callback Context | Timer task | Main loop |

### 📋 Required Loop Calls

**ESP32:**
```cpp
void loop() {
    GPIO_Scheduler.run();           // Required for scheduled callbacks
    GenericInput::processPCFIRQ();  // Required for PCF interrupts
}
```

**ESP8266:**
```cpp
void loop() {
    // No special calls required
    // Callbacks are automatically scheduled
}
```

### 🔧 Compiler Differences

**ESP32:** Supports all C++11 features including advanced lambda captures

**ESP8266:** Limited C++11 support, avoid complex lambda captures

---

## 📞 Support & Troubleshooting

### 🆘 Common Issues

1. **Callbacks not executing**: Ensure `GPIO_Scheduler.run()` is called on ESP32
2. **Memory leaks**: Use RAII principles, avoid global dynamic objects
3. **PCF not responding**: Check I2C wiring and addresses
4. **Timer exhaustion**: Limit number of simultaneous timed operations

### 📧 Getting Help

- **GitHub Issues**: [Report bugs](https://github.com/nht173/DeviceLib/issues)
- **Discussions**: [Ask questions](https://github.com/nht173/DeviceLib/discussions)
- **Email**: nhthai173@gmail.com

---

## 📋 Quick Reference: Callback Scheduling

| Parameter | Execution | Safety | Use Case | Performance |
|-----------|-----------|--------|----------|-------------|
| `schedule = true` | Next loop iteration | ✅ Thread-safe | **Default choice** | ~1-2ms delay |
| `schedule = false` | Immediate (ISR) | ⚠️ Limited functions | Time-critical | <1μs response |

**Rule of thumb:** Use `schedule = true` unless you need microsecond response time and understand ISR limitations.

---

**🎯 This API documentation covers DeviceLib v1.1.1**

*For release notes and migration guides, see [release-notes/](../release-notes/)*
