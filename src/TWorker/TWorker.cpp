#include "TWorker/TWorker.h"

namespace Worker
{
TWorker *TWorker::p_instance = NULL;
TaskHandle_t TWorker::h_task;
SemaphoreHandle_t TWorker::xTermMutex;
bool TWorker::is_terminate = false;
QueueHandle_t TWorker::queue;

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
            delete p_wt;
        }
    }
}

TWorker::TWorker()
{
    p_instance = this;
    xTermMutex = xSemaphoreCreateMutex();

    queue = xQueueCreate(4, sizeof(TWorkerTaskAsyncBase *));
    if (queue == NULL) {
        throw String("TWorker::TWorker(): ошибка создания очереди");
    }

    //xTaskCreatePinnedToCore(task, "TWorker::task", 2500, this,
    xTaskCreatePinnedToCore(task, "TWorker::task", 2000, this,
        (tskIDLE_PRIORITY + 2), &h_task, portNUM_PROCESSORS - 2);
}

TWorker::~TWorker()
{
    // Если мы не в состоянии завершения работы:
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    bool is = is_terminate;
    xSemaphoreGive(xTermMutex);

    if(!is && h_task)
    {
        vTaskDelete(h_task);
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
    if(xTaskGetCurrentTaskHandle() == h_task)
    {
        p->accept(p_instance);
        p->run();
        delete p;
    }
    else
    {
        xQueueSend(queue, &p, 0);
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
}