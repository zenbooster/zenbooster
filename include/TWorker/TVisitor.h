#pragma once

namespace Worker
{
class TWorkerTaskAsyncBase;
class TWorkerTaskTerminate;

class TVisitor
{
public:
    virtual void visit(TWorkerTaskAsyncBase *p) = 0;
	virtual void visit(TWorkerTaskTerminate *p) = 0;
	virtual ~TVisitor();
};
}