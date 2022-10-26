#pragma once
#include "TSleepMode.h"
#include <Arduino.h>

namespace Worker
{
using namespace SleepMode;

class TWorkerTaskTerminate;
class TWorkerTaskLog;
class TWorkerTaskLogVariadic;

class TVisitor
{
public:
	virtual void visit(TWorkerTaskTerminate *p) = 0;
    virtual void visit(TWorkerTaskLog *p) = 0;
    virtual void visit(TWorkerTaskLogVariadic *p) = 0;
	virtual ~TVisitor() {};
};

class TWorkerTaskBase
{
private:
    //
public:
    TWorkerTaskBase();
    virtual ~TWorkerTaskBase();

    virtual void accept(TVisitor *v) = 0;
    virtual void post_send(void) {};
    virtual void run(void) = 0;
};

class TWorkerTaskTerminate: public TWorkerTaskBase
{
private:
    TCbSleepFunction cb;
    bool is_reset;

public:
    TWorkerTaskTerminate(TCbSleepFunction cb, bool is_reset = false);

    void accept(TVisitor *v) {v->visit(this);};
    void run(void);
};

class TWorkerTaskLog: public TWorkerTaskBase
{
private:
    String text;

public:
    TWorkerTaskLog(String& text);

    void accept(TVisitor *v);
    void run(void);
};

class TWorkerTaskLogVariadic: public TWorkerTaskBase
{
private:
    String text;
    va_list ap;
    TaskHandle_t h_task;

public:
    TWorkerTaskLogVariadic(const char *fmt, va_list ap);

    void accept(TVisitor *v);
    void post_send(void);
    void run(void);
};

class TWorker: public TVisitor
{
private:
    static TaskHandle_t h_task;
    static SemaphoreHandle_t xTermMutex;
    static bool is_terminate; // reset или shutdown
    static QueueHandle_t queue;

	void visit(TWorkerTaskTerminate *p);
    void visit(TWorkerTaskLog *p);
    void visit(TWorkerTaskLogVariadic *p);

    static void task(void *p);

public:
    TWorker();
    ~TWorker();

    // отправить объект задачи, созданный с помощью new:
    static void send(TWorkerTaskBase *p);
    static const void printf(const char *fmt, ...);
};
}