#pragma once
#include <Arduino.h>
#include <memory>
#include "TSingleton.h"
#include "TTask.h"
#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TWorker/TWorkerTaskSyncBase.h"
#include "TWorker/TWorkerTaskSyncResBase.h"
#include "TWorker/TWorkerTaskTerminate.h"
#include "TWorker/TWorkerTaskLog.h"
#include "TWorker/TWorkerTaskLogVariadic.h"
#include "TWorker/TWorkerTaskSprintf.h"
#include "TWorker/TWorkerVisitor.h"

namespace Worker
{
using namespace Singleton;
using namespace Task;

class TWorker: public TSingleton<TWorker>
{
private:
    static TTask *p_task;
    static TWorkerVisitor worker_visitor;
    static QueueHandle_t queue;

    const char *get_class_name();

    static void task(void *p);

public:
    TWorker();
    ~TWorker();

    // отправить объект задачи, созданный с помощью new:
    static void send(TWorkerTaskAsyncBase *p);
    static void send(TWorkerTaskSyncBase *p);

    template <class T>
    static const T send(TWorkerTaskSyncResBase<T> *p);

    static void print(const String& text);
    static void println(const String& text);

    // Из ISR не вызывать, иначе WDT таймаут:
    template <class ... Args>
    static void printf(Args ... args);
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
    if(xTaskGetCurrentTaskHandle() == p_task->get_handle())
    {
        p->accept((TVisitor*)get_instance());
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
void TWorker::printf(Args ... args)
{
    send(new TWorkerTaskLogVariadic(args...));
}

template <class ... Args>
const shared_ptr<char> TWorker::sprintf(Args ... args)
{
    return send(new TWorkerTaskSprintf(args...));
}
}