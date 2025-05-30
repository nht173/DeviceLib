#include "GenericButton.h"

uint32_t GenericButton::_gcd(uint32_t a, uint32_t b) {
    return b == 0 ? a : _gcd(b, a % b);
}

void GenericButton::_autoAdjustIdleTime() {
    if (_default_idle_time <= _default_dbclick_time) {
        _default_idle_time += _default_dbclick_time;
        GI_DEBUG_PRINTF("[Button][%d] Idle time is less than click wait time. Set idle time to %u\n",
                        _pin, _default_idle_time);
    }
}

void GenericButton::_autoAdjustTickTime() {
    uint32_t gcdVal = _default_hold_time;
    for (auto &cb: _callbacks) {
        if (cb.event == BUTTON_EVENT_PRESS_HOLD) {
            gcdVal = _gcd(gcdVal, cb.param);
        }
    }
    _hold_time_tick = (gcdVal < 50) ? 50 : gcdVal;
    GI_DEBUG_PRINTF("[Button][%d] _hold_time_tick based onEvent gcd = %u\n", _pin, _hold_time_tick);
}

void GenericButton::_execCallback(generic_button_cb_t *cb) {
    if (cb == nullptr || cb->callback.fn == nullptr) return;
    if (cb->excuted) return;
    GenericInput::_execCallback(cb->callback);
    cb->excuted = true;
}

void GenericButton::_processHandler() {
    bool currentState = _read(true);
    if (currentState == _lastState) return;
    _lastState = currentState;
    if (currentState == _activeState) {
        _state = BUTTON_STATE_PRESSED;
        _last_press_time = millis();
        GI_DEBUG_PRINTF("[Button][%d][%lu] Pressed\n", _pin, millis());
        if (_click_count == 0)
            _click_count = 1;
        if (millis() - _last_release_time < _default_dbclick_time) {
            ++_click_count;
        }
/* hold event process */
#if defined(ESP8266)
        _ticker.attach_ms(_hold_time_tick, [this]() {
            _process_hold();
        });
#elif defined(ESP32)
        _btnTimerArg.callback = std::bind(&GenericButton::_process_hold, this);
                    if (_btnTimer != nullptr) {
                        esp_timer_stop(_btnTimer);
                        esp_timer_start_periodic(_btnTimer, _hold_time_tick * 1000);
                    }
#endif
/* press event process */
        _process_press();
    } else {
        _state = BUTTON_STATE_RELEASED;
        _last_release_time = millis();
        GI_DEBUG_PRINTF("[Button][%d][%lu] Released\n", _pin, millis());
/* timer to process click event */
#if defined(ESP8266)
        _ticker.once_ms(_default_dbclick_time, [this]() {
            _process_click();
/* timer to process idle */
            _ticker.once_ms(_default_idle_time - _default_dbclick_time, [this]() {
                _process_idle();
            });
        });
#elif defined(ESP32)
        _btnTimerArg.callback = [this]() {
                        _process_click();
                        /* timer to process idle */
                        if (_btnTimer != nullptr) {
                            _btnTimerArg.callback = std::bind(&GenericButton::_process_idle, this);
                            esp_timer_stop(_btnTimer);
                            esp_timer_start_once(_btnTimer, (_default_idle_time - _default_dbclick_time) * 1000);
                        }
                    };
                    if (_btnTimer != nullptr) {
                        esp_timer_stop(_btnTimer);
                        esp_timer_start_once(_btnTimer, _default_dbclick_time * 1000);
                    }
#endif
/* release event process */
        _process_release();
    }
}


void GenericButton::_process_press() {
    for (auto &cb: _callbacks) {
        if (cb.event == BUTTON_EVENT_IDLE || cb.event == BUTTON_EVENT_RELEASED) {
            // reset idle callback
            cb.excuted = false;
        } else if (cb.event == BUTTON_EVENT_PRESSED) {
            _execCallback(&cb);
        } else if (cb.event == BUTTON_EVENT_STATE_CHANGE) {
            cb.excuted = false; // reset state change callback
            _execCallback(&cb);
        }
    }
}


void GenericButton::_process_release() {
    for (auto &cb: _callbacks) {
        if (cb.event == BUTTON_EVENT_PRESSED) {
            cb.excuted = false; // reset pressed callback
        } else if (cb.event == BUTTON_EVENT_RELEASED) {
            _execCallback(&cb);
        } else if (cb.event == BUTTON_EVENT_STATE_CHANGE) {
            cb.excuted = false; // reset state change callback
            _execCallback(&cb);
        }
    }
}


void GenericButton::_process_click() {
    /* process click event */
    if (_click_count == 0) return;
    if (_last_release_time - _last_press_time >= _default_hold_time) {
        // ignore click event if hold event is triggered
        _click_count = 0;
        return;
    }
    GI_DEBUG_PRINTF("[Button][%d][%lu] Click count: %d\n", _pin, millis(), _click_count);
    for (auto &cb: _callbacks) {
        switch (_click_count) {
            case 1:
                if (cb.event == BUTTON_EVENT_CLICK || (cb.event == BUTTON_EVENT_CLICK_COUNT && cb.param == 1)) {
                    _execCallback(&cb);
                }
                break;
            case 2:
                if (cb.event == BUTTON_EVENT_DOUBLE_CLICK || (cb.event == BUTTON_EVENT_CLICK_COUNT && cb.param == 2)) {
                    _execCallback(&cb);
                }
                break;
            default:
                if (cb.event == BUTTON_EVENT_CLICK_COUNT && cb.param == _click_count) {
                    _execCallback(&cb);
                }
                break;
        }
    }
    _click_count = 0;
}


void GenericButton::_process_idle() {
    if (_last_release_time == 0) return;
    _state = BUTTON_STATE_IDLE;
    GI_DEBUG_PRINTF("[Button][%d][%lu] Idle\n", _pin, millis());
    for (auto &cb: _callbacks) {
        switch (cb.event) {
            case BUTTON_EVENT_IDLE:
                _execCallback(&cb);
                break;
            case BUTTON_EVENT_STATE_CHANGE:
                cb.excuted = false; // reset state change callback
                _execCallback(&cb);
                break;
            default:
                cb.excuted = false; // reset other callbacks
                break;
        }
    }
    _last_release_time = 0;
    _last_press_time = 0;
    _click_count = 0;
}


void GenericButton::_process_hold() {
    if (_last_press_time == 0) return;
    uint32_t hold_time = millis() - _last_press_time;
    GI_DEBUG_PRINTF("[Button][%d][%lu] Hold time: %lu ms\n", _pin, millis(), hold_time);
    for (auto &cb: _callbacks) {
        if (cb.excuted) continue;
        if (cb.event == BUTTON_EVENT_LONG_CLICK && hold_time >= _default_hold_time) {
            _execCallback(&cb);
        } else if (cb.event == BUTTON_EVENT_PRESS_HOLD && hold_time >= cb.param) {
            _execCallback(&cb);
        }
    }
}
