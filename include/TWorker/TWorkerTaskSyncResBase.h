#pragma once
#include "TWorker/TWorkerTaskSyncBase.h"
#include "TWorker/TVisitor.h"
#include <functional>

namespace Worker
{
using namespace std;

typedef function<void(void)> TCbWorkerTaskSyncResBase;

template<class T>
class TWorkerTaskSyncResBase: public TWorkerTaskSyncBase
{
protected:
    T res;
    TCbWorkerTaskSyncResBase cb;

public:
    TWorkerTaskSyncResBase() {};
    //virtual ~TWorkerTaskSyncResBase();

    virtual void accept(TVisitor *v)
    {
        v->visit(this);
    };

    virtual void run(void)
    {
        cb();
    }

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