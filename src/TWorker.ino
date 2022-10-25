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
    //
}

void TWorkerTaskLog::run(void)
{
    Serial.print(text);
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

////////////////
void TWorker::task(void *p)
{
    TWorker *p_this = (TWorker *)p;
    for(;;)
    {
        TWorkerTaskBase *p_wt = NULL;
        bool res = xQueueReceive(queue, &p_wt, portMAX_DELAY);
        //Serial.printf("TWorker::task(..): p_wt=%p\n", p_wt);

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
    //Serial.printf("TWorker::send(0x%p)\n", p);
    xQueueSend(queue, &p, 0);
}

//
// SerialPrintf
// Реализует функциональность printf в Serial.print
// Применяется для отладочной печати
// Параметры как у printf
// Возвращает 
//    0 - ошибка формата
//    отрицательное чило - нехватка памяти, модуль числа равен запрашиваемой памяти
//    положительное число - количество символов, выведенное в Serial
//
const size_t TWorker::printf(const char *szFormat, ...)
{
  va_list argptr;
  va_start(argptr, szFormat);
  char *szBuffer = 0;
  const size_t nBufferLength = vsnprintf(szBuffer, 0, szFormat, argptr) + 1;
  if (nBufferLength == 1) return 0;
  szBuffer = (char *) malloc(nBufferLength);
  if (! szBuffer) return - nBufferLength;
  vsnprintf(szBuffer, nBufferLength, szFormat, argptr);
  String text(szBuffer);
  send(new TWorkerTaskLog(text));
  free(szBuffer);
  return nBufferLength - 1;
} // const size_t SerialPrintf (const char *szFormat, ...)
}