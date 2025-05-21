//
// Created by Thái Nguyễn on 23/6/24.
//

#include "GenericInput.h"

#if defined(USE_PCF)
std::vector<pcf_irq_t> GenericInput::_pcfIRQ;
#ifdef ESP32
QueueHandle_t GenericInput::pcfIRQQueueHandle = nullptr;
#endif // ESP32
#endif // USE_PCF


GenericInput::GenericInput(uint8_t pin, uint8_t mode, bool activeState, uint32_t debounceTime, bool useInterrupt) {
    pinMode(pin, mode);
    _pin = pin;
    _activeState = activeState;
    _lastState = digitalRead(_pin);
    _debounceTime = debounceTime;
    if (useInterrupt) {
        attachInterrupt(CHANGE);
    }
}


#if defined(USE_PCF)

GenericInput::GenericInput(PCF_TYPE &pcf, uint8_t pin, uint8_t mode, bool activeState, uint32_t debounceTime,
                           bool useInterrupt) {
    _pcf = &pcf;
    _pin = pin;
    _setMode(mode);
    _activeState = activeState;
    _lastState = _pcf->digitalRead(_pin);
    _debounceTime = debounceTime;
    if (useInterrupt) {
        attachInterrupt(CHANGE);
    }
}

#endif


bool GenericInput::attachInterrupt(uint8_t mode) {
#if defined(ESP32)
    // Create timer for debounce
    esp_timer_create_args_t timerArgs = {
            .callback = reinterpret_cast<esp_timer_cb_t>(_debounceHandler),
            .arg = this,
            .name = String("gidt" + String(_pin)).c_str(),
    };
    esp_err_t err = esp_timer_create(&timerArgs, &_debounceTimer);
    if (err != ESP_OK) {
        Serial.printf("[Err][Create timer] Failed to create timer for pin[%d]\n", _pin);
        return false;
    }
#endif
#if defined(USE_PCF)
    if (_pcf != nullptr) {
        for (auto &pcfIRQ: _pcfIRQ) {
            if (pcfIRQ.pcf == _pcf) {
                pcfIRQ.inputs.push_back(this);
                return true;
            }
        }
        pcf_irq_t pcfIRQ;
        pcfIRQ.pcf = _pcf;
        pcfIRQ.inputs.push_back(this);
        _pcfIRQ.push_back(pcfIRQ);
        return true;
    }
#endif
    if (digitalPinToInterrupt(_pin) < 0)
        return false;
    detachInterrupt();
    ::attachInterruptArg(_pin, _irqHandler, this, mode);
    return true;
} // attachInterrupt

#if defined(USE_PCF)

bool GenericInput::attachInterrupt(PCF_TYPE *pcf, uint8_t boardPin) {
    if (pcf == nullptr || digitalPinToInterrupt(boardPin) < 0) {
        return false;
    }

#ifdef ESP32
    /* init queue */
    if (pcfIRQQueueHandle == nullptr) {
        pcfIRQQueueHandle = xQueueCreate(5, sizeof(pcf_irq_t *));
        if (pcfIRQQueueHandle == nullptr) {
            Serial.println("[Err][GenericInput::PCF] Failed to create queue");
            return false;
        }
    }
#endif

    /* Find pcf */
    pcf_irq_t *_attached = nullptr;
    for (auto &pcfIRQ: _pcfIRQ) {
        if (pcfIRQ.pcf == pcf) {
            _attached = &pcfIRQ;
//                pcfIRQ.attachedPin = boardPin;
            break;
        }
    }
    if (_attached == nullptr) {
        pcf_irq_t pcfIRQ;
        pcfIRQ.pcf = pcf;
        //        pcfIRQ.attachedPin = boardPin;
        _pcfIRQ.push_back(pcfIRQ);
        _attached = &_pcfIRQ.back();
    }
    pinMode(boardPin, INPUT_PULLUP);
    ::detachInterrupt(digitalPinToInterrupt(boardPin));
    ::attachInterruptArg(boardPin, _pcfIRQHandler, _attached, FALLING);
    return true;
}

#endif


void GenericInput::detachInterrupt() {
#if defined(USE_PCF)
    if (_pcf != nullptr) {
        if (!_pcfIRQ.empty()) {
            for (auto &pcfIRQ: _pcfIRQ) {
                if (pcfIRQ.pcf == _pcf && !pcfIRQ.inputs.empty()) {
                    for (auto it = pcfIRQ.inputs.begin(); it != pcfIRQ.inputs.end(); ++it) {
                        if (*it == this) {
                            pcfIRQ.inputs.erase(it);
                            break;
                        }
                    }
                    // @TODO detach interrupt if no more pins
                    return;
                }
            }
        }
        return;
    }
#endif
    ::detachInterrupt(digitalPinToInterrupt(_pin));
#if defined(ESP32)
    // delete timer
    if (_debounceTimer != nullptr) {
        esp_timer_stop(_debounceTimer);
        esp_timer_delete(_debounceTimer);
        _debounceTimer = nullptr;
    }
#endif
} // detachInterrupt




/* ================ ISR ================ */

IRAM_ATTR void GenericInput::_irqHandler(void *arg) {
    auto *self = (GenericInput *) arg;
#if defined(ESP32)
    if (self->_debounceTimer != nullptr) {
        esp_timer_stop(self->_debounceTimer);
        esp_timer_start_once(self->_debounceTimer, self->_debounceTime * 1000);
    }
#elif defined(ESP8266)
    self->_ticker.detach();
    if (self->_debounceTime > 0) {
        self->_ticker.once_ms(self->_debounceTime, _debounceHandler, self);
    } else {
        _debounceHandler(self);
    }
#endif
}


void GenericInput::_debounceHandler(GenericInput *pInput) {
    GI_DEBUG_PRINTF("[Debounce] pin[%d] state[%d]\n", pInput->_pin, pInput->_lastState);
    bool currentState = pInput->_read(true);
    if (currentState == pInput->_lastState)
        return;
    GI_DEBUG_PRINTF("\t -> pin[%d] %s\n", pInput->_pin, currentState == pInput->_activeState ? "ACTIVE" : "INACTIVE");
    pInput->_lastState = currentState;
    if (currentState == pInput->_activeState) {
        GI_DEBUG_PRINTF("[Callback][%d] Start ActiveCB\n", pInput->_pin);
        pInput->_execCallback(pInput->_onActiveCB);
        GI_DEBUG_PRINTF("[Callback][%d] End ActiveCB\n", pInput->_pin);
    } else {
        GI_DEBUG_PRINTF("[Callback][%d] Start InactiveCB\n", pInput->_pin);
        pInput->_execCallback(pInput->_onInactiveCB);
        GI_DEBUG_PRINTF("[Callback][%d] End InactiveCB\n", pInput->_pin);
    }
    GI_DEBUG_PRINTF("[Callback][%d] Start ChangeCB\n", pInput->_pin);
    pInput->_execCallback(pInput->_onChangeCB);
    GI_DEBUG_PRINTF("[Callback][%d] End ChangeCB\n", pInput->_pin);
}





/* ================ PCF ISR ================ */

IRAM_ATTR void GenericInput::_pcfIRQHandler(void *arg) {
    auto *pcfIRQ = (pcf_irq_t *) arg;
#if defined(ESP32)
    if (pcfIRQ && pcfIRQQueueHandle) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(pcfIRQQueueHandle, &pcfIRQ, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
#elif defined(ESP8266)

    GI_DEBUG_PRINTF("PCF IRQ [0x%02x]\n", pcfIRQ->pcf->getAddress());
    for (auto &input: pcfIRQ->inputs) {
        GI_DEBUG_PRINTF("\t -> pin[%d] %s\n", input->_pin, input->_read(true) == input->_activeState ? "ACTIVE" : "INACTIVE");
        if (input->_lastState == input->_read(true))
            continue;
        if (input->_debounceTime > 0) {
            GI_DEBUG_PRINTF("[Debounce][%d] Start debounce\n", input->_pin);
            input->_ticker.detach();
            input->_ticker.once_ms(input->_debounceTime, _debounceHandler, input);
        } else {
            _debounceHandler(input);
        }
    }

#endif // ESP8266
}


#if defined(ESP32)
void GenericInput::processPCFIRQ() {
    pcf_irq_t *pcfIRQ = nullptr;
    while (xQueueReceive(pcfIRQQueueHandle, &pcfIRQ, 0) == pdTRUE) {
        if (pcfIRQ == nullptr) continue;
        for (auto &input: pcfIRQ->inputs) {
            if (input->_lastState == input->_read(true))
                continue;
            if (input->_debounceTime > 0) {
                if (input->_debounceTimer != nullptr) {
                    esp_timer_stop(input->_debounceTimer);
                    esp_timer_start_once(input->_debounceTimer, input->_debounceTime * 1000);
                }
            } else {
                _debounceHandler(input);
            }
        }
    }
}
#endif // ESP32