#include "TWorker/TWorkerVisitor.h"

namespace Worker
{
TWorkerVisitor::TWorkerVisitor():
    is_terminate(false)
{
    xTermMutex = xSemaphoreCreateMutex();
}

TWorkerVisitor::~TWorkerVisitor()
{
    vSemaphoreDelete(xTermMutex);
}

void TWorkerVisitor::visit(TWorkerTaskAsyncBase *p)
{
    //
}

void TWorkerVisitor::visit(TWorkerTaskTerminate *p)
{
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    is_terminate = true;
    xSemaphoreGive(xTermMutex);
}

bool TWorkerVisitor::is_terminated()
{
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    bool res = is_terminate;
    xSemaphoreGive(xTermMutex);

    return res;
}
}