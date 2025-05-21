#ifndef GPIO_HELPER_H
#define GPIO_HELPER_H


/* Schedule for esp32 */
#if defined(ESP32)
#include "ScheduleRun.h"
extern ScheduleRun GPIO_Scheduler;
#else
#include <Schedule.h>
#endif





#endif //GPIO_HELPER_H
