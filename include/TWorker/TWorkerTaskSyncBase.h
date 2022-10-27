#pragma once
#include <Arduino.h>
#include "TWorker/TWorkerTaskAsyncBase.h"

namespace Worker
{
class TVisitor;

class TWorkerTaskSyncBase: public TWorkerTaskAsyncBase
{
private:
    TaskHandle_t h_task; // caller task

public:
    TWorkerTaskSyncBase();
    virtual ~TWorkerTaskSyncBase();

    virtual void accept(TVisitor *v);
    virtual void run(void) = 0;
    virtual void release(void);
};
}