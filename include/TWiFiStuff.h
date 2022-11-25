#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
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
    static TTask *p_task;
    static SemaphoreHandle_t x_dtor_mutex;
    static TaskHandle_t h_dtor_task;
    static WiFiUDP ntp_udp;
    static NTPClient time_cli;
    static int timeoffset;
    static TTgmBot *pTgmBot;
    static bool is_mqtt;
    static TMQTTClient *p_mqtt;
    static SemaphoreHandle_t x_mqtt_send_mutex;

    const char *get_class_name();

    static void task(void *p);
    static void wait_for_send();

    // для борьбы с неявной рекурсией:
    static bool is_mqtt_disabling;
    static void set_mqtt_active(bool is);

    static void setTimeOffset(int t);

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

    static unsigned long getUtcEpochTime();
    static unsigned long getEpochTime();
    static unsigned long UTC(unsigned long t);

    static bool is_mqtt_active();
    static void mqtt_send(const char *topic, const DynamicJsonDocument *p);
};
}
