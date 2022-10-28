
#include "TWorker/TWorker.h"
#include "TWorker/TWorkerTaskCmdGetConf.h"
//#include "TMyApplication.h"
#include "TUtil.h"
#include "TConf.h"
#include <ArduinoJson.h>
#include <StreamString.h>

namespace Worker
{
//using namespace MyApplication;
using namespace Util;
using namespace Conf;

TWorkerTaskCmdGetConf::TWorkerTaskCmdGetConf(void)
{
    cb = [this] (void)
    {
        DynamicJsonDocument doc = TConf::get_json();
        StreamString ss;
        serializeJson(doc, ss);
        res = TUtil::sprintf("`%s`", TUtil::screen_mark_down(ss.c_str()).get());
    };
}
}