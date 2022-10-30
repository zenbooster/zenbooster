#include "TTask.h"
#include "TTaskObserver.h"

namespace Task
{
TTask::TTask(TCbTaskFunction cb, const char *name, const size_t sz_stack, void *p_par, UBaseType_t priority):
    sz_stack(sz_stack),
    h_task(NULL)
{
    xTaskCreate(cb, name, sz_stack, p_par, priority, &h_task);
}

TTask::TTask(TCbTaskFunction cb, const char *name, const size_t sz_stack, void *p_par, UBaseType_t priority, const BaseType_t core_id):
    sz_stack(sz_stack),
    h_task(NULL)
{
    xTaskCreatePinnedToCore(cb, name, sz_stack, p_par,
        priority, &h_task, portNUM_PROCESSORS - 2);

    TTaskObserver::add_task(this);
}

TTask::~TTask()
{
    if(h_task)
    {
        TTaskObserver::del_task(this);
        vTaskDelete(h_task);
    }
}

TaskHandle_t TTask::get_handle()
{
    return h_task;
}

char *TTask::get_name()
{
    return pcTaskGetTaskName(h_task);
}

size_t TTask::GetMaxStackSizeUsage(void)
{
    return sz_stack - uxTaskGetStackHighWaterMark(h_task);
}
}