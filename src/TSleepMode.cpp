#include "TSleepMode.h"
#include "TConf.h"
#include "TMyApplication.h"
#include "TWorker/TWorker.h"
#include "common.h"

namespace SleepMode
{
using namespace Worker;

TCbSleepFunction TSleepMode::cb = NULL;
SemaphoreHandle_t TSleepMode::xGrMutex;
bool TSleepMode::is_graceful = true;
int64_t TSleepMode::first_press_time = 0;

#ifdef PIN_BTN
void TSleepMode::say_button_pressed()
{
    Serial.printf("Нажата кнопка на пине GPIO_NUM_%d.\n", sleep_pin);
}

void /*IRAM_ATTR*/ TSleepMode::isr_handle()
{
    if(!first_press_time)
    {
        say_button_pressed();
        Serial.println("(первый раз)");
        first_press_time = esp_timer_get_time();
    }
    else
    {
        if(esp_timer_get_time() - first_press_time >= 1000 * SLEEP_SECOND_PRESS_DELAY_MS)
        {
            say_button_pressed();
            Serial.println("(второй раз)");
            detachInterrupt(sleep_pin);

            Serial.println("Срочно спать...");
            Serial.flush();
            esp_deep_sleep_start();
        }
    }
    //detachInterrupt(sleep_pin);
    //Serial.printf("Нажата кнопка на пине GPIO_NUM_%d.\n", sleep_pin);

    shutdown();
}
#endif

TSleepMode::TSleepMode(TCbSleepFunction cb)
{
    TSleepMode::cb = cb;

    xGrMutex = xSemaphoreCreateMutex();

#ifdef PIN_BTN
    pinMode(sleep_pin, INPUT);
    
    int buttonState;
    for(;;)
    {
        buttonState = !digitalRead(sleep_pin);
        if(buttonState)
        {
            TWorker::printf("Клавиша на пине GPIO_NUM_%d всё ещё (?) нажата. Надо отпустить... )\n", sleep_pin);
            vTaskDelay(300);
            continue;
        }
        break;
    }

    pinMode(sleep_pin, INPUT_PULLUP);
    attachInterrupt(sleep_pin, isr_handle, RISING);
#endif
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