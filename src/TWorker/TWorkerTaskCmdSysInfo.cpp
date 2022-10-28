
#include "TWorker/TWorker.h"
#include "TWorker/TWorkerTaskCmdSysInfo.h"
#include "TMyApplication.h"
#include "TUtil.h"
#include <WiFi.h>
//#include "esp_arduino_version.h"

namespace Worker
{
using namespace MyApplication;
using namespace Util;

const shared_ptr<char> TWorkerTaskCmdSysInfo::ip2str(ip4_addr_t& ip)
{
  return TUtil::sprintf(IPSTR, IP2STR(&ip));
}

TWorkerTaskCmdSysInfo::TWorkerTaskCmdSysInfo(void)
{
    cb = [this] (void)
    {
        #ifdef ESP_ARDUINO_VERSION_MAJOR
        String s_ard_ver = 
            String(ESP_ARDUINO_VERSION_MAJOR, 10) + "\\." + 
            String(ESP_ARDUINO_VERSION_MINOR, 10) + "\\." +
            String(ESP_ARDUINO_VERSION_PATCH, 10);
        #endif
        tcpip_adapter_ip_info_t ipInfo;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

        const char fmt[] = 
            "*Система*:\n"
            "*версия IDF*: %s\n"
        #ifdef ESP_ARDUINO_VERSION_MAJOR
            "*версия Arduino*: %s\n"
        #endif
            "*прошивка*: %s\n"
            "*MAC адрес адаптера*: %s\n"
            "*IP адрес адаптера*: %s\n"
            "*IP адрес шлюза*: %s\n"
            "*reset\\_reason*: %d\n"
            "*размер свободной кучи*: %lu\n"
            "*максимальный размер свободного блока в куче*: %lu\n";

        res = TUtil::sprintf(
            fmt,
            TUtil::screen_mark_down(esp_get_idf_version()).get(),
        #ifdef ESP_ARDUINO_VERSION_MAJOR
            s_ard_ver.c_str(),
        #endif
            TUtil::screen_mark_down(TMyApplication::get_version_string().c_str()).get(),
            WiFi.macAddress().c_str(),
            TUtil::screen_mark_down(ip2str(ipInfo.ip)).get(),
            TUtil::screen_mark_down(ip2str(ipInfo.gw)).get(),
            esp_reset_reason(),
            heap_caps_get_free_size(MALLOC_CAP_8BIT),
            heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
        );
    };
}
}