#include "GenericOutput.h"

void GenericOutput::on(bool force)
{
    // on delay, timer will be reset if already running
    if (_pState != stdGenericOutput::ON && _pOnDelay > 0)
    {
        if (_pState != stdGenericOutput::WAIT_FOR_ON || force) {
            _pState = stdGenericOutput::WAIT_FOR_ON;
#if defined(ESP8266)
            _ticker.detach();
            _ticker.once_ms(_pOnDelay, _onTick, this);
#elif defined(ESP32)
            esp_timer_stop(_timer);
            esp_timer_start_once(_timer, _pOnDelay * 1000);
#endif
            return;
        }
    }
    // Turn on
    _on_function(force);
    // auto off, timer will be reset if already running
    if (_autoOffEnabled && _duration > 0)
    {
#if defined(ESP8266)
        if (!_ticker.active() || force) {
            _ticker.detach();
            _ticker.once_ms(_duration, _onTick, this);
        }
#elif defined(ESP32)
        if (_timer == nullptr || force) {
            esp_timer_stop(_timer);
            esp_timer_start_once(_timer, _duration * 1000);
        }
#endif
    }
}

void GenericOutput::on(uint32_t onDelay, uint32_t duration, bool force) {
    // on delay, timer will be reset if already running
    if (_pState != stdGenericOutput::ON && _pOnDelay > 0)
    {
        if (_pState != stdGenericOutput::WAIT_FOR_ON || force) {
            _pState = stdGenericOutput::WAIT_FOR_ON;
#if defined(ESP8266)
            _ticker.detach();
            _ticker.once_ms(onDelay, _onTick, this);
#elif defined(ESP32)
            esp_timer_stop(_timer);
            esp_timer_start_once(_timer, onDelay * 1000);
#endif
            return;
        }
    }
    // Turn on
    _on_function(force);
    // auto off, timer will be reset if already running
    if (_autoOffEnabled && _duration > 0)
    {
#if defined(ESP8266)
        if (!_ticker.active() || force) {
            _ticker.detach();
            _ticker.once_ms(duration, _onTick, this);
        }
#elif defined(ESP32)
        if (_timer == nullptr || force) {
            esp_timer_stop(_timer);
            esp_timer_start_once(_timer, duration * 1000);
        }
#endif
    }
}

void GenericOutput::onPercentage(uint8_t percentage, bool force) {
    if (percentage < 1 || percentage > 100) return;
    on(_pOnDelay, _duration * percentage / 100);
}

void GenericOutput::off(bool force)
{
#if defined(ESP8266)
    _ticker.detach();
    _off_function(force);
#elif defined(ESP32)
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
    _timer = nullptr;
#endif
}

void GenericOutput::setPowerOnDelay(uint32_t delay)
{
    _pOnDelay = delay;
}

void GenericOutput::setAutoOff(bool autoOffEnabled) {
    _autoOffEnabled = autoOffEnabled;
}

void GenericOutput::setAutoOff(bool autoOffEnabled, uint32_t duration) {
    _autoOffEnabled = autoOffEnabled;
    _duration = duration;
}

void GenericOutput::setDuration(uint32_t duration)
{
    _duration = duration;
    if (duration > 0)
        _autoOffEnabled = true;
    else
        _autoOffEnabled = false;
}

uint32_t GenericOutput::getDuration() const
{
    return _duration;
}

uint32_t GenericOutput::getPowerOnDelay() const {
    return _pOnDelay;
}