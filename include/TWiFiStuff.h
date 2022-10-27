#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "TTgmBot.h"

namespace Conf {class TConf;}
namespace MedSession {class TMedSession;}

namespace WiFiStuff
{
using namespace Conf;
using namespace TgmBot;
using namespace MedSession;

class TWiFiStuff
{
  private:
    static int ref_cnt;
    //static String dev_name;
    static TaskHandle_t h_task;
    static SemaphoreHandle_t xDtorMutex;
    static TaskHandle_t h_dtor_task;
    static WiFiUDP ntp_udp;
    static NTPClient time_cli;
    static TTgmBot *pTgmBot;

    static void task(void *p);

    friend class Conf::TConf;
    friend class MedSession::TMedSession;

  public:
    TWiFiStuff(String dev_name, TgmBot::TCbChangeFunction cb_change_formula);
    ~TWiFiStuff();

    static void tgb_send(const String& m, bool isMarkdownEnabled = true);
};
}
