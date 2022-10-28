#pragma once
#include <Arduino.h>
#include <memory>
#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TWorker/TWorkerTaskSyncBase.h"
#include "TWorker/TWorkerTaskSyncResBase.h"
#include "TWorker/TWorkerTaskTerminate.h"
#include "TWorker/TWorkerTaskLog.h"
#include "TWorker/TWorkerTaskLogVariadic.h"
#include "TWorker/TWorkerTaskSprintf.h"
#include "TWorker/TVisitor.h"

namespace Worker
{
class TWorker: public TVisitor
{
private:
    static TWorker *p_instance;
    static TaskHandle_t h_task;
    static SemaphoreHandle_t xTermMutex;
    static bool is_terminate; // reset или shutdown
    static QueueHandle_t queue;

    void visit(TWorkerTaskAsyncBase *p);
	void visit(TWorkerTaskTerminate *p);

    static void task(void *p);

public:
    TWorker();
    ~TWorker();

    // отправить объект задачи, созданный с помощью new:
    static void send(TWorkerTaskAsyncBase *p);
    static void send(TWorkerTaskSyncBase *p);

    template <class T>
    static const T send(TWorkerTaskSyncResBase<T> *p);

    static const void print(const String& text);
    static const void println(const String& text);

    // Из ISR не вызывать, иначе WDT таймаут:
    template <class ... Args>
    static const void printf(Args ... args);
    template <class ... Args>
    static const inline shared_ptr<char> sprintf(Args ... args);

    static const shared_ptr<char> screen_mark_down(const char *s);
    static const shared_ptr<char> screen_mark_down(const shared_ptr<char> s);
    static const shared_ptr<char> cmd_sysinfo(void);
    static const shared_ptr<char> cmd_getconf(void);
};

template <class T>
const T TWorker::send(TWorkerTaskSyncResBase<T> *p)
{
    T res;

    // Если задача посылается из задачи выполняющейся в данный
    // момент. Например, если из TWorkerTaskTerminate вызывается
    // колбек, использующий TWorker::printf, делаем такую проверку:
    if(xTaskGetCurrentTaskHandle() == h_task)
    {
        p->accept(p_instance);
        p->run();
        p->release();
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        res = p->dtor_result();
    }
    else
    {
        xQueueSend(queue, &p, 0);
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);\
        res = p->dtor_result();
    }

    return res;
}

template <class ... Args>
const void TWorker::printf(Args ... args)
{
    send(new TWorkerTaskLogVariadic(args...));
}

template <class ... Args>
const shared_ptr<char> TWorker::sprintf(Args ... args)
{
    return send(new TWorkerTaskSprintf(args...));
}
}