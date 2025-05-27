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
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    void setOnFunction(std::function<void()> onFunction, bool schedule = true) {
        _onFunction.assign(onFunction, schedule);
    }

    /**
     * @brief Set function callback for OFF state
     * @param offFunction
     * @param schedule if true, the callback will be scheduled to run in the next loop iteration
     *                 if false, the callback will be executed immediately
     */
    void setOffFunction(std::function<void()> offFunction, bool schedule = true) {
        _offFunction.assign(offFunction, schedule);
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
    devlib_callback_t _onFunction;
    devlib_callback_t _offFunction;

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
