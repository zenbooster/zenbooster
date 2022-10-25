#pragma once
#include "common.h"
#include <functional>

#ifdef PIN_BTN
namespace Conf {class TConf;}
namespace MyApplication {class TMyApplication;}

namespace SleepMode
{
using namespace Conf;
using namespace MyApplication;
using namespace std;

typedef function<void(void)> TCbSleepFunction;

class TSleepMode
{
    private:
        static const uint8_t sleep_pin = PIN_BTN;
        static TaskHandle_t h_task;
        static TCbSleepFunction cb;
        static bool is_graceful;
        static bool is_reset;

        static void IRAM_ATTR isr_handle();

        static void task(void *p) __attribute__ ((noreturn));

        friend class Conf::TConf;

    public:
        TSleepMode(TCbSleepFunction cb = 0);

        static void reset(void);
        static void shutdown(void);
};
}
#endif