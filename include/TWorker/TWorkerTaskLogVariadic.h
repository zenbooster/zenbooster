#pragma once
#include "TWorker/TWorkerTaskAsyncBase.h"
#include <Arduino.h>
#include <functional>

namespace Worker
{
using namespace std;

typedef function<void(void)> TCbWorkerTaskLogVariadic;

class TWorkerTaskLogVariadic: public TWorkerTaskAsyncBase
{
private:
    TCbWorkerTaskLogVariadic cb;

public:
    template <class ... Args>
    TWorkerTaskLogVariadic(Args ... args);

    void run(void);
};

template <class ... Args>
TWorkerTaskLogVariadic::TWorkerTaskLogVariadic(Args ... args)
{
    cb = [args...] (void)
    {
        Serial.printf(args...);
    };
}
}