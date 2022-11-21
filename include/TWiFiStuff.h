#pragma once
#include <Arduino.h>
#include "TSingleton.h"
#include "TTask.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "TTgmBot.h"
#include "TWorker/TWorkerTaskCmdSysInfo.h"

namespace Conf {class TConf;}
namespace MedSession {class TMedSession;}
namespace MQTTClient {class TMQTTClient;}

namespace WiFiStuff
{
using namespace Singleton;
using namespace Task;
using namespace Conf;
using namespace TgmBot;
using namespace MedSession;
using namespace MQTTClient;

class TWiFiStuff: public TSingleton<TWiFiStuff>
{
  private:
    //static int ref_cnt;
    //static String dev_name;
    //static TaskHandle_t h_task;
    static TTask *p_task;
    static SemaphoreHandle_t xDtorMutex;
    static TaskHandle_t h_dtor_task;
    static WiFiUDP ntp_udp;
    static NTPClient time_cli;
    static TTgmBot *pTgmBot;

    static bool is_mqtt;
    static TMQTTClient *p_mqtt;

    const char *get_class_name();

    static void task(void *p);

    friend class Conf::TConf;
    friend class MedSession::TMedSession;
    friend class Worker::TWorkerTaskCmdSysInfo;

  public:
    TWiFiStuff(String dev_name, TgmBot::TCbChangeFunction cb_change_formula);
    ~TWiFiStuff();

    static void tgb_send(const char *m, bool isMarkdownEnabled = true);
    static inline void tgb_send(const String& m, bool isMarkdownEnabled = true)
    {
      tgb_send(m.c_str(), isMarkdownEnabled);
    }
    static inline void tgb_send(const std::shared_ptr<char> m, bool isMarkdownEnabled = true)
    {
      tgb_send(m.get(), isMarkdownEnabled);
    }

    static unsigned long getEpochTime();
};
}
