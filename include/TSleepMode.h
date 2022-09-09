#pragma once
#include "common.h"

namespace SleepMode
{
    class TSleepMode
    {
        private:
            //static const uint8_t sleep_pin = GPIO_NUM_27;
            //static const uint8_t sleep_pin = GPIO_NUM_15;
            //static const uint8_t sleep_pin = GPIO_NUM_4;
            static const uint8_t sleep_pin = PIN_BTN;

            static void IRAM_ATTR isr_handle() __attribute__ ((noreturn));

        public:
            TSleepMode();
    };
}