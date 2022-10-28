#include "TWorker/TWorkerTaskScreenMarkDown.h"
#include "TUtil.h"

namespace Worker
{
using namespace Util;

TWorkerTaskScreenMarkDown::TWorkerTaskScreenMarkDown(const char *s)
{
    cb = [this, s] (void)
    {
        String t = TUtil::screen_mark_down(s).get();
        res = new char[t.length() + 1];
        strcpy(res, t.c_str());
    };
}
}