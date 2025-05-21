#include <Arduino.h>
#include "GenericOutput.h"

/* Basic usage with startup last state */
GenericOutput led(
    LED_BUILTIN, /* pin */
    LOW,         /* active state */
    stdGenericOutput::START_UP_LAST_STATE /* startup state */
);

/* LED auto off after 5 seconds */
// GenericOutput led(LED_BUILTIN, LOW, stdGenericOutput::START_UP_LAST_STATE, 5000);

void setup()
{
    Serial.begin(115200);

    led.setPowerOnDelay(1000); // 1 second delay before turning on

    led.begin(); // apply the last state

    // Callbacks
    led.onPowerChanged([]() {
        Serial.print("LED state changed to: ");
        Serial.println(led.getState() ? "ON" : "OFF");
    });

    led.onPowerOn([]() {
        Serial.println("LED is ON");
    });

    led.onPowerOff([]() {
        Serial.println("LED is OFF");
    });
}

void loop()
{
#ifdef ESP32
    GPIO_Scheduler.run();
#endif
}