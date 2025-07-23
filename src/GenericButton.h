#ifndef GENERIC_BUTTON_H
#define GENERIC_BUTTON_H

#include "DeviceLibTypes.h"
#include "GenericInput.h"
#include <vector>
#include <algorithm>

enum generic_button_event_t
{
    BUTTON_EVENT_IDLE = 0,
    BUTTON_EVENT_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_CLICK,
    BUTTON_EVENT_CLICK_COUNT,
    BUTTON_EVENT_PRESS_HOLD,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_STATE_CHANGE
};

enum generic_button_state_t
{
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASED
};

struct generic_button_cb_t
{
    generic_button_event_t event;
    devlib_callback_t callback;
    uint32_t param{};
    bool excuted = false;
    generic_button_cb_t() = default;
    generic_button_cb_t(generic_button_event_t evt, std::function<void()> cb, uint32_t p = 0, bool schedule = true)
        : event(evt), param(p), excuted(false) {
            callback.fn = std::move(cb);
            callback.schedule = schedule;
        }
};
 

class GenericButton : public GenericInput {
public:
    GenericButton() = default;

    /**
     * @brief Construct a new GenericButton object
     * @param pin pin number
     * @param mode pin mode. Default is INPUT
     * @param activeState active state. Default is LOW
     * @param debounceTime debounce time in milliseconds. Default is 50
     */
    explicit GenericButton(uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50)
        : GenericInput(pin, mode, activeState, debounceTime) {}

#if defined(USE_PCF)

    /**
     * @brief Construct a new GenericInput object
     * @param pcf PCF8574|PCF8575 object
     * @param pin pin number
     * @param mode pin mode. Default is INPUT
     * @param activeState active state. Default is LOW
     * @param debounceTime debounce time in milliseconds. Default is 50
     */
    explicit GenericButton(PCF_TYPE &pcf, uint8_t pin, uint8_t mode = INPUT, bool activeState = LOW, uint32_t debounceTime = 50)
        : GenericInput(pcf, pin, mode, activeState, debounceTime) {}

#endif

    /**
     * @brief Destructor - cleanup timers and callbacks safely
     */
    ~GenericButton() {
#if defined(ESP8266)
        // Stop ESP8266 Ticker
        _ticker.detach();
#elif defined(ESP32)
        // Stop ESP32 timer first to prevent callbacks during destruction
        if (_btnTimer != nullptr) {
            esp_timer_stop(_btnTimer);
            esp_timer_delete(_btnTimer);
            _btnTimer = nullptr;
        }
#endif
        // Clear callbacks explicitly for safety (though vector destructor would do this)
        _callbacks.clear();
    }

    /**
     * @brief Get the State object
     * 
     * @return generic_button_state_t 
     */
    generic_button_state_t getState() const {
        return _state;
    }

    /**
     * @brief Get the current state as a string.
     * 
     * @return String 
     */
    String getStateString() override {
        switch (_state) {
            case BUTTON_STATE_IDLE:
                return "idle";
            case BUTTON_STATE_PRESSED:
                return "pressed";
            case BUTTON_STATE_RELEASED:
                return "released";
            default:
                return "Unknown";
        }
    }

    /**
     * @brief Set wait time for multiple clicks.
     * 
     * @param time 
     */
    void setClickWaitTime(uint32_t time) {
        _default_dbclick_time = time;
        _autoAdjustIdleTime();
    }

    /**
     * @brief Get the wait time for multiple clicks.
     * 
     * @return uint32_t 
     */
    uint32_t getClickWaitTime() const {
        return _default_dbclick_time;
    }

    /**
     * @brief Set the idle time.
     * 
     * @param time 
     */
    void setIdleTime(uint32_t time) {
        _default_idle_time = time;
        _autoAdjustIdleTime();
    }

    /**
     * @brief Get the idle time.
     * 
     * @return uint32_t 
     */
    uint32_t getIdleTime() const {
        return _default_idle_time;
    }

    /**
     * @brief Set the hold time.
     * 
     * @param time 
     */
    void setHoldTime(uint32_t time) {
        if (time <= 2 * _default_dbclick_time) {
            time = 2 * _default_dbclick_time;
            GI_DEBUG_PRINTF("[Button][%d] Hold time is less than double click time. Set hold time to %u\n",
                _pin, 2 * _default_dbclick_time);
        }
        _default_hold_time = time;
        _autoAdjustTickTime();
    }

    /**
     * @brief Get the hold time.
     * 
     * @return uint32_t 
     */
    uint32_t getHoldTime() const {
        return _default_hold_time;
    }


    /* ========================== Callbacks ========================== */

    /**
     * @deprecated Use onRelease instead
     * @brief [Deprecated]. Use onRelease instead.
     * 
     * @param cb callback function
     * @param schedule whether to schedule the callback
     */
    [[deprecated("Use onRelease instead")]]
    void onInactive(std::function<void()> cb, bool schedule = true) override {
        onRelease(std::move(cb), schedule);
    }

    /**
     * @deprecated Use onPress instead
     * @brief [Deprecated]. Use onPress instead.
     * 
     * @param cb callback function
     * @param schedule whether to schedule the callback
     */
    [[deprecated("Use onPress instead")]]
    void onActive(std::function<void()> cb, bool schedule = true) override {
        onClick(std::move(cb), schedule);
    }

    /**
     * @brief Add a callback function for a specific button event.
     * 
     * @param event 
     * @param cb 
     * @param param if the event is BUTTON_EVENT_CLICK_COUNT or BUTTON_EVENT_PRESS_HOLD,
     * this parameter will be used to specify the count of clicks or hold time in milliseconds.
     */
    void onEvent(generic_button_event_t event, std::function<void()> cb, uint32_t param = 0, bool schedule = true) {
        generic_button_cb_t listener(event, std::move(cb), param, schedule);
        _callbacks.push_back(std::move(listener));
        _init();
    }

    /**
     * @brief The callback function will be executed when the button state is changed. (Pressed, Released, Idle)
     * 
     * @param cb 
     */
    void onChange(std::function<void()> cb, bool schedule = true) override {
        onEvent(BUTTON_EVENT_STATE_CHANGE, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is pressed.
     * 
     * @param cb 
     */
    void onPress(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_PRESSED, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is released.
     * 
     * @param cb 
     */
    void onRelease(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_RELEASED, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is idle.
     * 
     * @param cb 
     */
    void onIdle(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_IDLE, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is click.
     * 
     * @param cb 
     */
    void onClick(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_CLICK, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is double click.
     * 
     * @param cb 
     */
    void onDoubleClick(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_DOUBLE_CLICK, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is long click.
     * 
     * @param cb 
     */
    void onLongClick(std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_LONG_CLICK, std::move(cb), 0, schedule);
    }

    /**
     * @brief The callback function will be executed when the count of click is equal to count.
     * 
     * @param count
     * @param cb 
     */
    void onClickCount(uint8_t count, std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_CLICK_COUNT, std::move(cb), count, schedule);
    }

    /**
     * @brief The callback function will be executed when the button is holded for hold_time.
     * 
     * @param hold_time in milliseconds
     * @param cb 
     */
    void onPressHold(uint32_t hold_time, std::function<void()> cb, bool schedule = true) {
        onEvent(BUTTON_EVENT_PRESS_HOLD, std::move(cb), hold_time, schedule);
        _autoAdjustTickTime();
    }

    /**
     * @brief Clear all callbacks for this button
     */
    void clearCallbacks() {
        _callbacks.clear();
    }

    /**
     * @brief Remove callbacks for a specific event type
     * @param event Event type to remove
     */
    void removeCallbacks(generic_button_event_t event) {
        _callbacks.erase(
            std::remove_if(_callbacks.begin(), _callbacks.end(),
                [event](const generic_button_cb_t& cb) {
                    return cb.event == event;
                }),
            _callbacks.end()
        );
    }

protected:
    generic_button_state_t _state = BUTTON_STATE_IDLE;
    uint32_t _default_dbclick_time = 300;
    uint32_t _default_idle_time = 500;
    uint32_t _default_hold_time = 3000;
    uint32_t _hold_time_tick = _default_hold_time; // how often to check hold time in milliseconds
    uint32_t _last_press_time = 0;
    uint32_t _last_release_time = 0;
    uint8_t _click_count = 0;
    std::vector<generic_button_cb_t> _callbacks;
#if defined(ESP32)
    esp_timer_handle_t _btnTimer = nullptr;

    struct btn_timer_args_t {
        GenericButton *button;
        std::function<void()> callback;
    } _btnTimerArg;

    static void _espTimerCallback(void* arg) {
        if (arg) {
            auto cb = static_cast<btn_timer_args_t*>(arg)->callback;
            if (cb) cb();
        }
    }

    void _init() override {
        if (_isInitialized) return;
        _btnTimerArg.button = this;
        _btnTimerArg.callback = nullptr;
        esp_timer_create_args_t timerArgs = {
            .callback = &_espTimerCallback,
            .arg = &_btnTimerArg,
            .name = String("gbtn" + String(_pin)).c_str(),
        };
        esp_err_t err = esp_timer_create(&timerArgs, &_btnTimer);
        if (err != ESP_OK) {
            GI_DEBUG_PRINTF("[Button][%d] Failed to create button timer: %s\n", _pin, esp_err_to_name(err));
            _btnTimer = nullptr;
        }
        GenericInput::_init();
    }
#endif

    static uint32_t _gcd(uint32_t a, uint32_t b);

    void _autoAdjustIdleTime();

    void _autoAdjustTickTime();

    void _execCallback(generic_button_cb_t *cb);

    void _processHandler() override;

    virtual void _process_press();

    virtual void _process_release();

    virtual void _process_click();

    virtual void _process_idle();

    virtual void _process_hold();

};

#endif // GENERIC_BUTTON_H