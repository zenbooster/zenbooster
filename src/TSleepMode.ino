#include "TSleepMode.h"

#ifdef PIN_BTN
#include "TConf.h"
#include "TMyApplication.h"
#include "TWorker.h"

namespace SleepMode
{
using namespace Worker;

TCbSleepFunction TSleepMode::cb = NULL;
SemaphoreHandle_t TSleepMode::xGrMutex;
bool TSleepMode::is_graceful = true;

void /*IRAM_ATTR*/ TSleepMode::isr_handle()
{
    detachInterrupt(sleep_pin);
    Serial.printf("Нажата кнопка на пине GPIO_NUM_%d.\n", sleep_pin);

    shutdown();
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

    xGrMutex = xSemaphoreCreateMutex();

    pinMode(sleep_pin, INPUT_PULLUP);
    attachInterrupt(sleep_pin, isr_handle, RISING);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)sleep_pin, 0);
}

void TSleepMode::terminate(bool is_reset)
{
    BaseType_t xHigherPriorityTaskWoken = 0;
    xSemaphoreTakeFromISR(xGrMutex, &xHigherPriorityTaskWoken);
    bool is = is_graceful;
    xSemaphoreGiveFromISR(xGrMutex, &xHigherPriorityTaskWoken);
    TWorker::send(new TWorkerTaskTerminate(is ? cb : NULL, is_reset));
}

void TSleepMode::reset(void)
{
    terminate(true);
}

void TSleepMode::shutdown(void)
{
    terminate(false);
}
}
#endif