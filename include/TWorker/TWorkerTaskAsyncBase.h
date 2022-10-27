#pragma once

namespace Worker
{
class TVisitor;

class TWorkerTaskAsyncBase
{
private:
    //
public:
    TWorkerTaskAsyncBase();
    virtual ~TWorkerTaskAsyncBase();

    virtual void accept(TVisitor *v);
    virtual void run(void) = 0;
};
}