//#ifndef VIRTUALOUTPUT_H
//#define VIRTUALOUTPUT_H
//
//#include <Arduino.h>
//#include <Ticker.h>
//#include <vector>
//
//// Schedule for esp32
//#if defined(ESP32)
//#include "ScheduleRun.h"
//#ifndef GPIO_SCHEDULER
//extern ScheduleRun GPIO_Scheduler;
//#endif // GPIO_SCHEDULER
//#else
//#include <Schedule.h>
//#endif
//
//namespace stdGenericOutput {
//    class VirtualOutput;
//    typedef struct {
//        String name;
//        std::function<void()> impl_fn;
//        uint32_t delayStart;
//        uint32_t duration;
//        std::function<void()> onStateStart;
//        std::function<void()> onStateActive;
//        std::function<void()> onStateEnd;
//    } state_impl_t;
//}
//
//class VirtualOutput {
//public:
//    VirtualOutput() = default;
//
//    void addState(const String& name, std::function<void()> impl_fn, uint32_t delayStart = 0, uint32_t duration = 0) {
//        stdGenericOutput::state_impl_t stateImpl;
//        stateImpl.name = name;
//        stateImpl.impl_fn = impl_fn;
//        stateImpl.delayStart = delayStart;
//        stateImpl.duration = duration;
//        _stateImpls.push_back(stateImpl);
//    }
//
//    void removeState(const String& name) {
//        auto it = std::remove_if(_stateImpls.begin(), _stateImpls.end(), [&](const stdGenericOutput::state_impl_t& stateImpl) {
//            return stateImpl.name == name;
//        });
//        _stateImpls.erase(it, _stateImpls.end());
//    }
//
//    void setState(const String& name) {
//        for (auto& stateImpl : _stateImpls) {
//            if (stateImpl.name == name) {
//                if (_onStateChanged != nullptr) {
//
//                }
//                _currentState = stateImpl.name;
//                stateImpl.impl_fn();
//                break;
//            }
//        }
//    }
//
//protected:
//    stdGenericOutput::state_impl_t* _currentState = nullptr;
//    std::vector<stdGenericOutput::state_impl_t> _stateImpls;
//    std::vector<stdGenericOutput::state_impl_t*> _stateCycleThrough;
//    std::function<void(const String oldState, const String newState)> _onStateChanged = nullptr;
//};
//
//
//#endif //VIRTUALOUTPUT_H
