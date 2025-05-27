#ifndef DEVICE_LIB_TYPES_H
#define DEVICE_LIB_TYPES_H

#include <functional>

#define DEVICE_LIB_VERSION "1.0.0"
#define DEVICE_LIB_VERSION_NUM 1

struct devlib_callback_t {
    std::function<void()> fn = nullptr;
    bool schedule = false;

    devlib_callback_t() = default;
    explicit devlib_callback_t(std::function<void()> callback, bool schedule = true)
        : fn(std::move(callback)), schedule(schedule) {}
    bool isValid() const {
        return fn != nullptr;
    }
    void assign(std::function<void()> &callback, bool schedule = true) {
        fn = std::move(callback);
        this->schedule = schedule;
    }
    void operator()() const {
        if (fn != nullptr) {
            fn();
        }
    }
};

#endif // DEVICE_LIB_TYPES_H