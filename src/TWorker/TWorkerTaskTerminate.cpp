#include "TWorker/TWorkerTaskTerminate.h"

namespace Worker
{
TWorkerTaskTerminate::TWorkerTaskTerminate(TCbSleepFunction cb, bool is_reset):
    cb(cb),
    is_reset(is_reset)
{
    //
}

void TWorkerTaskTerminate::run(void)
{
    if(cb)
    {
        cb();
    }

    if(is_reset)
    {
        Serial.println("Перезагружаемся...");
        Serial.flush();
        esp_restart();
    }
    else
    {
        Serial.println("Идём баиньки...");
        Serial.flush();
        esp_deep_sleep_start();
    }
}
}