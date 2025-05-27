//
// Created by Thái Nguyễn on 23/6/24.
//

#ifndef GENERICINPUT_H
#define GENERICINPUT_H

#include <Arduino.h>
#include "DeviceLibTypes.h"

// #define DEBUG

#ifdef DEBUG
#define GI_DEBUG_PRINTF(...) { Serial.print("[GenericInput] "); Serial.printf(__VA_ARGS__); }
#else
#define GI_DEBUG_PRINTF(...)
#endif // DEBUG


#if defined(ESP32)

#include <freertos/queue.h>
#include <esp_timer.h>
#include "GPIO_helper.h"

#elif defined(ESP8266)

#include <Ticker.h>
#include <functional>

#endif

class GenericInput;

#if __has_include_next(<PCF8574.h>)

#include <PCF8574.h>

#ifndef USE_PCF
#define USE_PCF
#define PCF_TYPE PCF8574
#endif // USE_PCF
#endif // __has_include_next(<PCF8574.h>)


#if defined(USE_PCF)

#include <vector>

struct pcf_irq_t {
    PCF_TYPE *pcf = nullptr;
    std::vector<GenericInput *> inputs = {};
//    int16_t attachedPin = -1;
};
#endif // USE_PCF

class GenericInput {

public:
    GenericInput() = default;

    /**
     * @brief Construct a new GenericButton object
     * @param pin pin number
     * @param mode pin mode. Default is INPUT
     * @param activeState active state. Default is LOW
     * @param debounceTime debounce time in milliseconds. Default is 50
     */
    explicit GenericInput(uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

#if defined(USE_PCF)

    /**
     * @brief Construct a new GenericInput object
     * @param pcf PCF8574|PCF8575 object
     * @param pin pin number
     * @param mode pin mode. Default is INPUT
     * @param activeState active state. Default is LOW
     * @param debounceTime debounce time in milliseconds. Default is 50
     */
    explicit GenericInput(PCF_TYPE &pcf, uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50);

#endif

    /**
     * @brief Set the pin number
     * @param pin
     */
    void setPin(uint8_t pin) {
        _pin = pin;
        _init();
    }

    /**
     * @brief Get the pin number
     * @return uint8_t
     */
    uint8_t getPin() const {
        return _pin;
    }

    /**
     * @brief Set the pin mode
     * @param mode
     */
    void setMode(uint8_t mode) {
        _setMode(mode);
        _init();
    }

    /**
     * @brief Set the debounce time
     * @param debounceTime
     */
    void setDebounceTime(uint32_t debounceTime) {
        _debounceTime = debounceTime;
        _init();
    }

    /**
     * @brief Get the debounce time
     * @return uint32_t
     */
    uint32_t getDebounceTime() const {
        return _debounceTime;
    }

    /**
     * @brief Set the active state
     * @param activeState
     */
    void setActiveState(bool activeState) {
        _activeState = activeState;
        _init();
    }

    /**
     * @brief Get the active state
     * @return true HIGH
     * @return false LOW
     */
    bool getActiveState() const {
        return _activeState;
    }

    /**
     * @brief Get the current state of the device
     * @return true when active
     * @return false when inactive
     */
    bool getState() {
        return _read() == _activeState;
    }

    /**
     * @brief Get the current state of the device as a string
     * 
     * @return String 
     */
    virtual String getStateString() {
        return getState() ? _activeStateStr : _inactiveStateStr;
    }

    /**
     * @brief set the state label
     * @param activeStateStr
     * @param inactiveStateStr
     */
    void setStateString(const String &activeStateStr, const String &inactiveStateStr) {
        _activeStateStr = activeStateStr;
        _inactiveStateStr = inactiveStateStr;
        _init();
    }

    /* ================== Callbacks ================== */

    /**
     * @brief Set the callback function when state changed
     * @param cb
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    virtual void onChange(std::function<void()> cb, bool schedule = true) {
        _onChangeCB.assign(cb, schedule);
        _init();
    }

    /**
     * @brief Set the callback function when active
     * @param cb
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    virtual void onActive(std::function<void()> cb, bool schedule = true) {
        _onActiveCB.assign(cb, schedule);
        _init();
    }

    /**
     * @brief Set the callback function when inactive
     * @param cb
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    virtual void onInactive(std::function<void()> cb, bool schedule = true) {
        _onInactiveCB.assign(cb, schedule);
        _init();
    }

    /**
     * @brief attach interrupt
     * @param mode
     * @return
     */
    bool attachInterrupt(uint8_t mode);

    /**
     * @brief detach interrupt
     */
    void detachInterrupt();

#if defined(USE_PCF)

    /**
     * @brief attach PCF interrupt
     * @param pcf
     * @param boardPin the INT pin of PCF connected to board
     * @return
     */
    static bool attachInterrupt(PCF_TYPE *pcf, uint8_t boardPin);

#if defined(ESP32)
    /**
     * @brief Call in loop to process PCF interrupt
     */
    static void processPCFIRQ();
#endif // ESP32

#endif // USE_PCF

protected:
    uint8_t _pin;
    bool _isInitialized = false;
    bool _lastState;
    bool _activeState;
    uint32_t _debounceTime;
    String _activeStateStr = "ACTIVE";
    String _inactiveStateStr = "NONE";
#if defined(ESP32)
    esp_timer_handle_t _timer = nullptr;
#elif defined(ESP8266)
    Ticker _ticker;
#endif
    // Callbacks
    devlib_callback_t _onChangeCB;
    devlib_callback_t _onActiveCB;
    devlib_callback_t _onInactiveCB;

    /**
     * @brief Initialize the input
     * 
     */
    virtual void _init();

    virtual void _execCallback(devlib_callback_t &cb) {
        if (!cb.isValid()) return;
        if (cb.schedule) {
#if defined(ESP32)
            GPIO_Scheduler.addSchedule(cb.fn);
#elif defined(ESP8266)
            schedule_function(cb.fn);
#endif
        } else {
            cb();
        }
    }

#if defined(USE_PCF)
    PCF_TYPE *_pcf = nullptr;
    static std::vector<pcf_irq_t> _pcfIRQ; // for PCF interrupt
#ifdef ESP32
    static QueueHandle_t pcfIRQQueueHandle; // for PCF interrupt
#endif

    IRAM_ATTR void static _pcfIRQHandler(void *arg);

#endif // USE_PCF

    /**
     * @brief Interrupt handler
     * @param arg GenericInput object
     */
    IRAM_ATTR void static _irqHandler(void *arg);

    /**
     * @brief Handler after debounce time
     * @param pInput GenericInput object
     */
    void static _debounceHandler(GenericInput *pInput);

    /**
     * @brief Input process handler
     * 
     */
    virtual void _processHandler();

    /**
 * @brief pinMode wrapper
 */
    void _setMode(uint8_t mode) {
#if defined(USE_PCF)
        if (_pcf != nullptr) {
            _pcf->pinMode(_pin, mode);
            return;
        }
#endif
        pinMode(_pin, mode);
    } // _setMode

    /**
     * @brief digitalRead wrapper
     */
    uint8_t _read(bool forceRead = false) {
#if defined(USE_PCF)
        if (_pcf != nullptr) {
            return _pcf->digitalRead(_pin, forceRead);
        }
#endif
        return digitalRead(_pin);
    } // _read

}; // class GenericInput


#endif //GENERICINPUT_H
