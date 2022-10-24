#include "TSleepMode.h"

#ifdef PIN_BTN
#include "TMyApplication.h"

namespace SleepMode
{
TaskHandle_t TSleepMode::h_task = NULL;
TCbSleepFunction TSleepMode::cb = NULL;

void /*IRAM_ATTR*/ TSleepMode::isr_handle()
{
    detachInterrupt(sleep_pin);
    Serial.printf("Нажата кнопка на пине GPIO_NUM_%d.\n", sleep_pin);

    BaseType_t xHigherPriorityTaskWoken = 0;
    xTaskNotifyFromISR(h_task, 0, eNoAction, &xHigherPriorityTaskWoken);
}

void TSleepMode::task(void *p)
{
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    if(TSleepMode::cb)
    {
        TSleepMode::cb();
    }

    Serial.println("Идём баиньки...");
    Serial.flush();
    ets_delay_us(100 * 1000); // борьба с дребезгом
    esp_deep_sleep_start();
}

TSleepMode::TSleepMode(TCbSleepFunction cb)
{
    TSleepMode::cb = cb;

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

    xTaskCreatePinnedToCore(task, "TSleepMode::task", 1500, this,
        (tskIDLE_PRIORITY + 2), &h_task, portNUM_PROCESSORS - 2);

    pinMode(sleep_pin, INPUT_PULLUP);
    attachInterrupt(sleep_pin, isr_handle, RISING);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)sleep_pin, 0);
}
}
#endif