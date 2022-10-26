#include "TWorker.h"

namespace Worker
{
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
TWorkerTaskLog::TWorkerTaskLog(String& text):
    text(text)
{
}

void TWorkerTaskLog::accept(TVisitor *v)
{
    v->visit(this);
}

void TWorkerTaskLog::run(void)
{
    Serial.print(text);
}

////////////////
TWorkerTaskLogVariadic::TWorkerTaskLogVariadic(const char *fmt, va_list ap):
    text(fmt)
{
    va_copy(this->ap, ap);
    h_task = xTaskGetCurrentTaskHandle();
}

void TWorkerTaskLogVariadic::accept(TVisitor *v)
{
    v->visit(this);
}

void TWorkerTaskLogVariadic::post_send(void)
{
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
}

void TWorkerTaskLogVariadic::run(void)
{
    char *buf = NULL;
    const char *fmt = text.c_str();
    const size_t nBufferLength = vsnprintf(buf, 0, fmt, ap) + 1;
    if (nBufferLength > 1)
    {
        buf = new char[nBufferLength];
        vsnprintf(buf, nBufferLength, fmt, ap);
        delete [] buf;
    }
    va_end(ap);
    Serial.println("TWorkerTaskLogVariadic::run(..): HIT.1");
    xTaskNotify(h_task, 0, eNoAction);
    Serial.println("TWorkerTaskLogVariadic::run(..): HIT.2");
}

////////////////
void TWorker::visit(TWorkerTaskTerminate *p)
{
    xSemaphoreTake(xTermMutex, portMAX_DELAY);
    is_terminate = true;
    xSemaphoreGive(xTermMutex);

    p->run();
    delete p;
}

void TWorker::visit(TWorkerTaskLog *p)
{
    p->run();
    delete p;
}

void TWorker::visit(TWorkerTaskLogVariadic *p)
{
    p->run();
    delete p;
}

////////////////
void TWorker::task(void *p)
{
    TWorker *p_this = (TWorker *)p;
    for(;;)
    {
        TWorkerTaskBase *p_wt = NULL;
        bool res = xQueueReceive(queue, &p_wt, portMAX_DELAY);
        Serial.printf("TWorker::task(..): p_wt=%p\n", p_wt);

        if(res)
        {
            p_wt->accept(p_this);
        }
    }
}

TWorker::TWorker()
{
    xTermMutex = xSemaphoreCreateMutex();

    queue = xQueueCreate(4, sizeof(TWorkerTaskBase *));
    if (queue == NULL) {
        throw String("TWorker::TWorker(): error creating the queue");
    }

    xTaskCreatePinnedToCore(task, "TWorker::task", 2500, this,
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
    Serial.printf("TWorker::send(0x%p)\n", p);
    xQueueSend(queue, &p, 0);
    Serial.println("TWorker::send(..): HIT.1");
    p->post_send();
    Serial.println("TWorker::send(..): HIT.2");
}

const void TWorker::printf(const char *fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
  send(new TWorkerTaskLogVariadic(fmt, argptr));
  va_end(argptr);
}
}