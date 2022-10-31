#pragma once
#include "TWorker/TVisitor.h"
#include <Arduino.h>

namespace Worker
{
class TWorkerVisitor: public TVisitor
{
private:
    SemaphoreHandle_t xTermMutex;
    bool is_terminate; // reset или shutdown

public:
    TWorkerVisitor();
    ~TWorkerVisitor();

    void visit(TWorkerTaskAsyncBase *p);
	void visit(TWorkerTaskTerminate *p);

    bool is_terminated();
};
}