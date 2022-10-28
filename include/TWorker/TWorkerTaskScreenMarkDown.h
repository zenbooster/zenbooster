#pragma once
#include "TWorker/TWorkerTaskSyncResBase.h"

namespace Worker
{
class TWorkerTaskScreenMarkDown: public TWorkerTaskSyncResBase<char *>
{
public:
    TWorkerTaskScreenMarkDown(const char *s);
};
}