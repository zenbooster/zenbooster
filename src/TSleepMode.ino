#include "TSleepMode.h"

namespace SleepMode
{
    void /*IRAM_ATTR*/ TSleepMode::isr_handle()
    {
        Serial.printf("Нажата клавиша на пине GPIO_NUM_%d.\n", sleep_pin);
        Serial.printf("Идём баиньки...\n");

        esp_deep_sleep_start();
    }

    TSleepMode::TSleepMode()
    {
        pinMode(sleep_pin, INPUT);
        
        int buttonState;
        for(;;)
        {
            buttonState = !digitalRead(sleep_pin);
            if(buttonState)
            {
                Serial.printf("Клавиша на пине GPIO_NUM_%d всё ещё (?) нажата. Надо отпустить... )\n", sleep_pin);
                vTaskDelay(300);
                continue;
            }
            break;
        }
        
        pinMode(sleep_pin, INPUT_PULLUP);
        attachInterrupt(sleep_pin, isr_handle, RISING);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)sleep_pin, 0);
    }
}