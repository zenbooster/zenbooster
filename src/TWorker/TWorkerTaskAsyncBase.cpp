#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TWorker/TVisitor.h"

namespace Worker
{
TWorkerTaskAsyncBase::TWorkerTaskAsyncBase()
{
    //
}

TWorkerTaskAsyncBase::~TWorkerTaskAsyncBase()
{
    //
}

void TWorkerTaskAsyncBase::accept(TVisitor *v)
{
    v->visit(this);
}

void TWorkerTaskAsyncBase::release(void)
{
    delete this;
}
}