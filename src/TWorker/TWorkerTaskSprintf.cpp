#include "TWorker/TWorkerTaskSprintf.h"

namespace Worker
{
TWorkerTaskSprintf::~TWorkerTaskSprintf()
{
}

void TWorkerTaskSprintf::run(void)
{
    cb();
}
}