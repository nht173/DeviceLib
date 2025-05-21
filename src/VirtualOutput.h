//
// Created by Thái Nguyễn on 3/5/25.
//

#include "GenericOutput.h"

#ifndef VIRTUALOUTPUT_H
#define VIRTUALOUTPUT_H


class VirtualOutput : public stdGenericOutput::GenericOutput {

public:

    VirtualOutput() : stdGenericOutput::GenericOutput() {}

#ifdef USE_LAST_STATE
    VirtualOutput(const String& name, stdGenericOutput::startup_state_t startUpState) : stdGenericOutput::GenericOutput() {
        _startUpState = startUpState;
        _pinKey = "v" + name;
    }
#endif

    /**
     * @brief Set function callback for ON state
     * @param onFunction
     */
    void setOnFunction(std::function<void()> onFunction) {
        _onFunction = std::move(onFunction);
    }

    /**
     * @brief Set function callback for OFF state
     * @param offFunction
     */
    void setOffFunction(std::function<void()> offFunction) {
        _offFunction = std::move(offFunction);
    }

    /**
     * @brief Set the label for ON state and OFF state
     * @param onStateStr
     * @param offStateStr
     */
    void setStateString(const String& onStateStr, const String& offStateStr) {
        _onStateStr = onStateStr;
        _offStateStr = offStateStr;
    }

    /**
     * @brief Get the state string
     * @return String
     */
    String getStateString() const override {
        return _state ? _onStateStr : _offStateStr;
    }

private:
    String _onStateStr = "ON";
    String _offStateStr = "OFF";
    std::function<void()> _onFunction = nullptr;
    std::function<void()> _offFunction = nullptr;

    void _on_function(bool force) override {
        _pState = stdGenericOutput::ON;
        _state = true;
        _execCallback(_onFunction);
        GenericOutput::on(force);
    }

    void _off_function(bool force) override {
        _pState = stdGenericOutput::OFF;
        _state = false;
        _execCallback(_offFunction);
        GenericOutput::off(force);
    }

};


#endif //VIRTUALOUTPUT_H
