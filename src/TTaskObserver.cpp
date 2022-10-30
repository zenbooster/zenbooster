#include "TTaskObserver.h"
#include "TTask.h"
#include "TUtil.h"

namespace Task
{
using namespace Util;

set<TTask*> TTaskObserver::tasks;

TTaskObserver::TTaskObserver()
{
    //
}

TTaskObserver::~TTaskObserver()
{
    //
}

void TTaskObserver::add_task(TTask *pt)
{
    tasks.insert(pt);
}

void TTaskObserver::del_task(TTask *pt)
{
    tasks.erase(pt);
}

shared_ptr<char> TTaskObserver::GetMaxStackSizeUsage(void)
{
    String s;

    for(auto t : tasks)
    {
        char *name = t->get_name();
        shared_ptr<char> spc = TUtil::sprintf("Задача *\\\"%s\\\"*\\: %lu байт стека\\.\n",
            name,
            (unsigned long)t->GetMaxStackSizeUsage()
        );
        char *pc = spc.get();
        s += pc;
    }

    char *res(new char[s.length() + 1]);
    strcpy(res, s.c_str());

    return shared_ptr<char>(res);
}
}