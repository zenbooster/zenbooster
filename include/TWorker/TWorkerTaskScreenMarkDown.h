#pragma once
#include "TWorker/TWorkerTaskSyncResBase.h"
#include <functional>
#include <memory>

namespace Worker
{
using namespace std;

typedef function<void(void)> TCbWorkerTaskSprintf;

class TWorkerTaskScreenMarkDown: public TWorkerTaskSyncResBase<char *>
{
private:
    TCbWorkerTaskSprintf cb;

public:
    TWorkerTaskScreenMarkDown(const char *s);
    ~TWorkerTaskScreenMarkDown();

    void run(void);
};
}