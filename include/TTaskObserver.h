#pragma once
#include "TSingleton.h"
#include <set>
#include <memory>

namespace Task
{
using namespace std;
using namespace Singleton;

class TTask;

class TTaskObserver: public TSingleton<TTaskObserver>
{
private:
    static set<TTask*> tasks;

public:
    TTaskObserver();
    ~TTaskObserver();

    static void add_task(TTask *pt);
    static void del_task(TTask *pt);
    static shared_ptr<char> GetMaxStackSizeUsage(void);
};
}