#ifndef SCHEDULERUN_H
#define SCHEDULERUN_H

#include "Arduino.h"
#include <vector>

class ScheduleRun {
private:
    uint8_t MAX_SCHEDULES = 0;
    std::vector<std::function<void()>> schedules;
public:

    /**
     *
     * @param maxSchedules 0 for unlimited schedules
     */
    ScheduleRun(uint8_t maxSchedules = 0) {
        MAX_SCHEDULES = maxSchedules;
    }

    ~ScheduleRun() {
        schedules.clear();
    }

    /**
     * @brief Add a schedule to the list
     * @param schedule
     */
    void addSchedule(std::function<void()> schedule) {
        if (MAX_SCHEDULES > 0 && schedules.size() >= MAX_SCHEDULES) {
            return;
        }
        schedules.push_back(schedule);
    }

    /**
     * @brief Run all schedules and remove them from the list
     */
    void run() {
        if (schedules.empty()) return;

        auto pending = schedules;
        schedules.clear();

        for (auto &schedule: pending) {
            if (schedule != nullptr) {
                schedule();
            }
        }
    }
};


#endif //SCHEDULERUN_H
