#pragma once
#include <Arduino.h>
#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TWorker/TWorkerTaskTerminate.h"
#include "TWorker/TWorkerTaskLog.h"
#include "TWorker/TWorkerTaskLogVariadic.h"
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

    static const void print(const String& text);
    static const void println(const String& text);

    template <class ... Args>
    static const void printf(Args ... args);
};

template <class ... Args>
const void TWorker::printf(Args ... args)
{
    send(new TWorkerTaskLogVariadic(args...));
}
}