#pragma once
#include "TWorker/TWorkerTaskAsyncBase.h"
#include <Arduino.h>

namespace Worker
{
class TWorkerTaskLog: public TWorkerTaskAsyncBase
{
private:
    String text;

public:
    TWorkerTaskLog(const String& text);

    void run(void);
};
}