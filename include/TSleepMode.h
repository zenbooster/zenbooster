#pragma once
#include "common.h"

namespace SleepMode
{
#ifdef PIN_BTN
    class TSleepMode
    {
        private:
            static const uint8_t sleep_pin = PIN_BTN;

            static void IRAM_ATTR isr_handle() __attribute__ ((noreturn));

        public:
            TSleepMode();
    };
#endif
}