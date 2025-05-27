#ifndef SCHEDULERUN_H
#define SCHEDULERUN_H

#include "Arduino.h"
#if defined(ESP32)
#include <freertos/queue.h>
#endif

#define MAX_SCHEDULES 20

class ScheduleRun {
private:
#if defined(ESP8266)
    std::function<void()> scheduleArr[MAX_SCHEDULES];
#elif defined(ESP32)
    QueueHandle_t scheduleQueue;
#endif
public:

    /**
     *
     * @param maxSchedules 0 for unlimited schedules
     */
    ScheduleRun() {
#if defined(ESP32)
        scheduleQueue = xQueueCreate(MAX_SCHEDULES, sizeof(std::function<void()>));
#endif
    }

    ~ScheduleRun() {
#if defined(ESP32)
        if (scheduleQueue != nullptr) {
            vQueueDelete(scheduleQueue);
            scheduleQueue = nullptr;
        }
#elif defined(ESP8266)
        for (auto &s : scheduleArr) {
            s = nullptr; // clear the schedule array
        }
#endif
    }

    /**
     * @brief Add a schedule to the list
     * @param schedule
     */
    void addSchedule(std::function<void()> schedule) {
#if defined(ESP32)
        if (scheduleQueue == nullptr) return;
        if (xQueueSend(scheduleQueue, &schedule, 0) != pdTRUE) {
            Serial.println("[Err][ScheduleRun] Failed to add schedule to queue");
        }
#elif defined(ESP8266)
        for (auto &s : scheduleArr) {
            if (s == nullptr) {
                s = schedule;
                return;
            }
        }
#endif
    }

    /**
     * @brief Run all schedules and remove them from the list
     */
    void run() {
#if defined(ESP32)
        if (scheduleQueue == nullptr) return;
        std::function<void()> schedule;
        while (xQueueReceive(scheduleQueue, &schedule, 0) == pdTRUE) {
            if (schedule != nullptr) {
                schedule(); // run the schedule
            }
        }
#elif defined(ESP8266)
        std::function<void()> pending[20];
        uint8_t count = 0;
        for (auto &schedule : scheduleArr) {
            if (schedule != nullptr) {
                pending[count++] = schedule;
                schedule = nullptr; // clear the schedule after running
            }
        }
        for (uint8_t i = 0; i < count; i++) {
            if (pending[i] != nullptr) {
                pending[i]();
            }
        }
#endif
    }
};


#endif //SCHEDULERUN_H
