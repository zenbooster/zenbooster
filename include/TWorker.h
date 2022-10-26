#pragma once
#include "TSleepMode.h"
#include <Arduino.h>

namespace Worker
{
using namespace SleepMode;

class TVisitor;

class TWorkerTaskBase
{
private:
    //
public:
    TWorkerTaskBase();
    virtual ~TWorkerTaskBase();

    virtual void accept(TVisitor *v);
    virtual void run(void) = 0;
};

class TWorkerTaskTerminate: public TWorkerTaskBase
{
private:
    TCbSleepFunction cb;
    bool is_reset;

public:
    TWorkerTaskTerminate(TCbSleepFunction cb, bool is_reset = false);

    void run(void);
};

class TWorkerTaskLog: public TWorkerTaskBase
{
private:
    String text;

public:
    TWorkerTaskLog(const String& text);

    void run(void);
};

typedef function<void(void)> TCbWorkerTaskLogVariadic;
class TWorkerTaskLogVariadic: public TWorkerTaskBase
{
private:
    TCbWorkerTaskLogVariadic cb;

public:
    template <class ... Args>
    TWorkerTaskLogVariadic(Args ... args);

    void run(void);
};

class TVisitor
{
public:
    virtual void visit(TWorkerTaskBase *p) = 0;
	virtual void visit(TWorkerTaskTerminate *p) = 0;
	virtual ~TVisitor() {};
};

class TWorker: public TVisitor
{
private:
    static TWorker *p_instance;
    static TaskHandle_t h_task;
    static SemaphoreHandle_t xTermMutex;
    static bool is_terminate; // reset или shutdown
    static QueueHandle_t queue;

    void visit(TWorkerTaskBase *p);
	void visit(TWorkerTaskTerminate *p);

    static void task(void *p);

public:
    TWorker();
    ~TWorker();

    // отправить объект задачи, созданный с помощью new:
    static void send(TWorkerTaskBase *p);
    template <class ... Args>
    static const void printf(Args ... args);
    static const void print(const String& text);
    static const void println(const String& text);
};
}