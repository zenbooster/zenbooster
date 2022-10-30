#include "common.h"
#include "TWorker/TWorker.h"
#include "TWorker/TWorkerTaskScreenMarkDown.h"
#include "TWorker/TWorkerTaskCmdSysInfo.h"
#include "TWorker/TWorkerTaskCmdGetConf.h"

namespace Worker
{
TTask *TWorker::p_task = NULL;
SemaphoreHandle_t TWorker::xTermMutex;
bool TWorker::is_terminate = false;
QueueHandle_t TWorker::queue;

const char *TWorker::get_class_name()
{
    return "TWorker";
}

void TWorker::visit(TWorkerTaskAsyncBase *p)
{
    //
}

void TWorker::visit(TWorkerTaskTerminate *p)
{
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    is_terminate = true;
    xSemaphoreGive(xTermMutex);
}

void TWorker::task(void *p)
{
    TWorker *p_this = (TWorker *)p;
    for(;;)
    {
        TWorkerTaskAsyncBase *p_wt = NULL;
        bool res = xQueueReceive(queue, &p_wt, portMAX_DELAY);

        if(res)
        {
            p_wt->accept(p_this);
            p_wt->run();
            p_wt->release();
        }
    }
}

TWorker::TWorker()
{
    xTermMutex = xSemaphoreCreateMutex();

    queue = xQueueCreate(4, sizeof(TWorkerTaskAsyncBase *));
    if (queue == NULL) {
        throw String("TWorker::TWorker(): ошибка создания очереди");
    }

    p_task = new TTask(task, "TWorker", TWORKER_TASK_STACK_SIZE, this, tskIDLE_PRIORITY + 2, portNUM_PROCESSORS - 2);
}

TWorker::~TWorker()
{
    // Если мы не в состоянии завершения работы:
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    bool is = is_terminate;
    xSemaphoreGive(xTermMutex);

    if(!is && p_task)
    {
        delete p_task;
    }

    if(queue)
    {
        vQueueDelete(queue);
    }

    vSemaphoreDelete(xTermMutex);
}

void TWorker::send(TWorkerTaskAsyncBase *p)
{
    // Если задача посылается из задачи выполняющейся в данный
    // момент. Например, если из TWorkerTaskTerminate вызывается
    // колбек, использующий TWorker::printf, делаем такую проверку:
    if(xTaskGetCurrentTaskHandle() == p_task->get_handle())
    {
        p->accept((TVisitor*)get_instance());
        p->run();
        delete p;
    }
    else
    {
        xQueueSend(queue, &p, 0);
    }
}

void TWorker::send(TWorkerTaskSyncBase *p)
{
    // Если задача посылается из задачи выполняющейся в данный
    // момент. Например, если из TWorkerTaskTerminate вызывается
    // колбек, использующий TWorker::printf, делаем такую проверку:
    if(xTaskGetCurrentTaskHandle() == p_task->get_handle())
    {
        p->accept((TVisitor*)get_instance());
        p->run();
        p->release();
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); // просто очищаем уведомление (сработает ?)
        delete p;
    }
    else
    {
        xQueueSend(queue, &p, 0);
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        delete p;
    }
}

const void TWorker::print(const String& text)
{
    send(new TWorkerTaskLog(text));
}

const void TWorker::println(const String& text)
{
    print(text + "\n");
}

const shared_ptr<char> TWorker::screen_mark_down(const char *s)
{
    return shared_ptr<char>(send(new TWorkerTaskScreenMarkDown(s)));
}

const shared_ptr<char> TWorker::screen_mark_down(const shared_ptr<char> s)
{
    return screen_mark_down(s.get());
}

const shared_ptr<char> TWorker::cmd_sysinfo(void)
{
    return send(new TWorkerTaskCmdSysInfo());
}

const shared_ptr<char> TWorker::cmd_getconf(void)
{
    return send(new TWorkerTaskCmdGetConf());
}
}