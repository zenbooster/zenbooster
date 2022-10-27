#pragma once
#include "TWorker/TWorkerTaskSyncBase.h"
#include "TWorker/TVisitor.h"

namespace Worker
{
template<class T>
class TWorkerTaskSyncResBase: public TWorkerTaskSyncBase
{
protected:
    T res;

public:
    TWorkerTaskSyncResBase() {};
    //virtual ~TWorkerTaskSyncResBase();

    virtual void accept(TVisitor *v)
    {
        v->visit(this);
    };
    virtual void run(void) = 0;

    T dtor_result(void);
};

// получаем результат и удаляем объект
template<class T>
T TWorkerTaskSyncResBase<T>::dtor_result(void)
{
    T res = this->res;
    delete this;

    return res;
}
}