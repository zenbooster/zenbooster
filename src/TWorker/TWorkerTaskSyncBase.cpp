#include "TWorker/TWorkerTaskSyncBase.h"
#include "TWorker/TVisitor.h"

namespace Worker
{
TWorkerTaskSyncBase::TWorkerTaskSyncBase()
{
    h_task = xTaskGetCurrentTaskHandle();
}

TWorkerTaskSyncBase::~TWorkerTaskSyncBase()
{
}

void TWorkerTaskSyncBase::accept(TVisitor *v)
{
    v->visit(this);
}

void TWorkerTaskSyncBase::release(void)
{
    xTaskNotify(h_task, 0, eNoAction);
}
}