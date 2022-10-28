#pragma once
#include "TWorker/TWorkerTaskSyncResBase.h"
#include <memory>

namespace Worker
{
using namespace std;

class TWorkerTaskCmdGetConf: public TWorkerTaskSyncResBase<shared_ptr<char>>
{
public:
    TWorkerTaskCmdGetConf(void);
};
}