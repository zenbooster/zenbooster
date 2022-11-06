#pragma once
#include "common.h"
#include <Arduino.h>
#include <functional>

namespace Conf {class TConf;}
namespace MyApplication {class TMyApplication;}
namespace Worker {class TWorker;}

namespace SleepMode
{
using namespace Conf;
using namespace MyApplication;
using namespace Worker;
using namespace std;

typedef function<void(void)> TCbSleepFunction;

class TSleepMode
{
    private:
        static TCbSleepFunction cb;
        static SemaphoreHandle_t xGrMutex;
        static bool is_graceful;
        static int64_t first_press_time;

    #ifdef PIN_BTN
        static const uint8_t sleep_pin = PIN_BTN;

        static void say_button_pressed();
        static void IRAM_ATTR isr_handle();
    #endif

        static void terminate(bool is_reset);

        friend class Conf::TConf;

    public:
        TSleepMode(TCbSleepFunction cb = 0);

        static void reset(void);
        static void shutdown(void);
};
}