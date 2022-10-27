#include "TWorker.h"

namespace Worker
{
TWorker *TWorker::p_instance = NULL;
TaskHandle_t TWorker::h_task;
SemaphoreHandle_t TWorker::xTermMutex;
bool TWorker::is_terminate = false;
QueueHandle_t TWorker::queue;

////////////////
TWorkerTaskBase::TWorkerTaskBase()
{
    //
}

TWorkerTaskBase::~TWorkerTaskBase()
{
    //
}

void TWorkerTaskBase::accept(TVisitor *v)
{
    v->visit(this);
}

////////////////
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

////////////////
TWorkerTaskLog::TWorkerTaskLog(const String& text):
    text(text)
{
}

void TWorkerTaskLog::run(void)
{
    Serial.print(text);
}

////////////////
void TWorkerTaskLogVariadic::run(void)
{
    cb();
}

////////////////
void TWorker::visit(TWorkerTaskBase *p)
{
    //
}

void TWorker::visit(TWorkerTaskTerminate *p)
{
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    is_terminate = true;
    xSemaphoreGive(xTermMutex);
}

////////////////
void TWorker::task(void *p)
{
    TWorker *p_this = (TWorker *)p;
    for(;;)
    {
        TWorkerTaskBase *p_wt = NULL;
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

    queue = xQueueCreate(4, sizeof(TWorkerTaskBase *));
    if (queue == NULL) {
        throw String("TWorker::TWorker(): error creating the queue");
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

void TWorker::send(TWorkerTaskBase *p)
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