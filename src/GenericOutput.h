#ifndef GENERIC_OUTPUT_H
#define GENERIC_OUTPUT_H

#include "GenericOutputBase.h"

#if defined(ESP8266)
#include <Ticker.h>
#elif defined(ESP32)
#include <esp_timer.h>
#endif

namespace stdGenericOutput {

    typedef enum {
        OFF = 0x00,
        ON = 0x01,
        WAIT_FOR_ON = 0x02,
    } state_t;

    class GenericOutput;
}

class stdGenericOutput::GenericOutput : public stdGenericOutput::GenericOutputBase {
public:

    GenericOutput() = default;


    /**
     * @brief Construct a new Auto Off object
     *
     * @param pin pin number
     * @param activeState LOW or HIGH. Default is LOW
     * @param duration duration to turn off after power is on in milliseconds
     */
    explicit GenericOutput(uint8_t pin, bool activeState = LOW,
                           startup_state_t startupState = stdGenericOutput::START_UP_NONE, uint32_t duration = 0)
            : GenericOutputBase(pin, activeState, startupState) {
        _autoOffEnabled = false;
        _duration = duration;
        if (duration > 0)
            _autoOffEnabled = true;
    }

#if defined(USE_PCF)

    /**
     * @brief Construct a new Auto Off object
     *
     * @param pcf PCF object
     * @param pin pin number
     * @param activeState LOW or HIGH. Default is LOW
     * @param duration duration to turn off after power is on in milliseconds
     */
    explicit GenericOutput(PCF_TYPE &pcf, uint8_t pin, bool activeState = LOW,
                           startup_state_t startupState = stdGenericOutput::START_UP_NONE, uint32_t duration = 0)
            : GenericOutputBase(pcf, pin, activeState, startupState) {
        _autoOffEnabled = false;
        _duration = duration;
        if (duration > 0)
            _autoOffEnabled = true;
    }

#endif

    void begin() override {
        GenericOutputBase::begin();
        esp_timer_create_args_t timerArgs = {
                .callback = reinterpret_cast<esp_timer_cb_t>(_onTick),
                .arg = this,
                .name = String("got" + String(_pin)).c_str(),
        };
        esp_err_t err = esp_timer_create(&timerArgs, &_timer);
        if (err != ESP_OK) {
            Serial.printf("[GenericOutput][Err][Create timer] Failed to create timer for pin[%d]\n", _pin);
        }
    }

    /**
     * @brief set power on
     *
     * This function turns on the output. If the `force` parameter is not specified, it defaults to `false`.
     *
     * Example:
     * @code
     * #include <GenericOutput.h>
     * // GenericOutput output(13) // basic initialization
     * GenericOutput output(13, HIGH, 5000); // Initialize with pin 13, active state HIGH, auto-off duration 5000ms
     * output.on(); // Turn on the output, it will stay on for 5 seconds
     * output.onDuration(2000); // Turn on the output for 2 seconds (only once)
     * output.onDuration(0); // Turn on the output for infinite time (only once)
     * @endcode
     *
     * @return void
     */
    void on() override {
        on(false);
    }

    /**
     * @brief set power on
     *
     */
    void on(bool force) override;

    /**
     * @brief set power on with custom delay and duration. Only works once
     * @param onDelay milliseconds to wait before turning on
     * @param duration milliseconds to wait before turning off, 0 to turn on indefinitely
     * @param force
     */
    void on(uint32_t onDelay, uint32_t duration, bool force = true);

    /**
     * @brief set power on with percentage timing (0-100%) based on the duration
     * @param percentage 1-100
     */
    void onPercentage(uint8_t percentage, bool force = false);

    /**
     * @brief set power off immediately
     *
     */
    void off(bool force) override;

    /**
     * @brief set power off immediately
     *
     */
    void off() override {
        off(false);
    }

    /**
     * @brief set a delay before power on. Set to 0 to disable
     *
     * @param delay milliseconds
     */
    void setPowerOnDelay(uint32_t delay);

    /**
     * @brief enable or disable auto off
     * @param autoOffEnabled true to enable, false to disable
     */
    void setAutoOff(bool autoOffEnabled);

    /**
     * @brief enable or disable auto off and set the duration
     *
     * @param autoOffEnabled
     * @param duration
     */
    void setAutoOff(bool autoOffEnabled, uint32_t duration);

    /**
     * @brief Set the duration to turn off after the power is on
     *
     * @param duration
     */
    void setDuration(uint32_t duration);

    /**
     * @brief Get the Duration object
     *
     * @return uint32_t
     */
    uint32_t getDuration() const;

    /**
     * @brief Get the Power On Delay object
     *
     * @return uint32_t
     */
    uint32_t getPowerOnDelay() const;

    /**
     * @brief Set the callback function to be called when power is turned off automatically
     *
     * @param onAutoOff callback function
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    void onAutoOff(std::function<void()> onAutoOff, bool schedule = true) {
        _onAutoOff.assign(onAutoOff, schedule);
    }

protected:
    bool _autoOffEnabled = false;
    state_t _pState = stdGenericOutput::OFF;
    uint32_t _duration = 0;
    uint32_t _pOnDelay = 0;
    devlib_callback_t _onAutoOff;
#if defined(ESP8266)
    Ticker _ticker;
#elif defined(ESP32)
    esp_timer_handle_t _timer = nullptr;
#endif

    virtual void _on_function(bool force) {
        _pState = stdGenericOutput::ON;
        GenericOutputBase::on(force);
    }

    virtual void _off_function(bool force) {
        _pState = stdGenericOutput::OFF;
        GenericOutputBase::off(force);
    }

    /**
     * @brief Ticker callback handler
     * @param pOutput
     */
    static void _onTick(GenericOutput *pOutput) {
        if (pOutput->_pState == stdGenericOutput::WAIT_FOR_ON) {
            pOutput->_pState = stdGenericOutput::ON;
            pOutput->on(true);
        } else if (pOutput->_pState == stdGenericOutput::ON && pOutput->_autoOffEnabled) {
            pOutput->off(true);
            pOutput->_execCallback(pOutput->_onAutoOff);
        }
    }
};


using stdGenericOutput::GenericOutput;

#endif //GENERIC_OUTPUT_H
