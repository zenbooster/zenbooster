#pragma once
#include "TWorker/TWorkerTaskSyncResBase.h"
//#include <tcpip_adapter.h>
#include <esp_netif.h>
#include <memory>

namespace Worker
{
using namespace std;

class TWorkerTaskCmdSysInfo: public TWorkerTaskSyncResBase<shared_ptr<char>>
{
private:
    static const shared_ptr<char> ip2str(ip4_addr_t& ip);

public:
    TWorkerTaskCmdSysInfo(void);
};
}