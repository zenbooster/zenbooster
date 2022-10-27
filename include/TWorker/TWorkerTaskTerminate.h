#pragma once
#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TSleepMode.h"

namespace Worker
{
using namespace SleepMode;

class TWorkerTaskTerminate: public TWorkerTaskAsyncBase
{
private:
    TCbSleepFunction cb;
    bool is_reset;

public:
    TWorkerTaskTerminate(TCbSleepFunction cb, bool is_reset = false);

    void run(void);
};
}