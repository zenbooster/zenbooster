#pragma once
#include "TWorker/TWorkerTaskSyncResBase.h"
#include <functional>

namespace Worker
{
using namespace std;

typedef function<void(void)> TCbWorkerTaskSprintf;

class TWorkerTaskSprintf: public TWorkerTaskSyncResBase<String>
{
private:
    TCbWorkerTaskSprintf cb;

public:
    template <class ... Args>
    TWorkerTaskSprintf(Args ... args);
    ~TWorkerTaskSprintf();

    void run(void);
};

template <class ... Args>
TWorkerTaskSprintf::TWorkerTaskSprintf(Args ... args)
{
    cb = [this, args...] (void)
    {
        //Serial.printf(args...);
        //String res;
        char *buf = 0;
        const size_t sz = snprintf(buf, 0, args...) + 1;
        //if (nBufferLength == 1) return 0;
        buf = new char[sz];
        //if(!buf) return -sz;
        snprintf(buf, sz, args...);
        res = buf;
        delete [] buf;
    };
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
/*const size_t TWorker::printf(const char *szFormat, ...)
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
*/
}