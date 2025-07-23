#include <Arduino.h>
#include "PCF8574_library.h"    // https://github.com/nht173/PCF8574_library.git
#include "GenericOutput.h"
#include "GenericInput.h"

#define ADDR 0x38
#define INT_PIN 16
#define LED1_PIN LED_BUILTIN
#define LED2_PIN 0
#define BUTTON_PIN 1

PCF8574 pcf(0x38); // I2C address of PCF8574

GenericOutput led1(LED1_PIN, LOW, stdGenericOutput::START_UP_LAST_STATE);
GenericOutput led2(pcf, LED2_PIN, LOW, stdGenericOutput::START_UP_LAST_STATE); // PCF8574 pin 0
GenericInput button(pcf, BUTTON_PIN, LOW); // PCF8574 pin 1 - active low
// INT pin from pcf8574 to D2


void setup()
{
    Serial.begin(115200);  // Initialize serial
    pcf.begin();                // Initialize PCF8574

    // Attach interrupt for pcf board to read button
    GenericInput::attachInterrupt(&pcf, INT_PIN);

    // apply the last state
    led1.begin();
    led2.begin();

    // Toggle LED on button pressed
    button.onActive([](){
        led1.toggle();
        led2.toggle();
    });
}

void loop()
{
#ifdef ESP32
    processPCFIRQ();
    GPIO_Scheduler.run();
#endif
}