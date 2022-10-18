#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "TConf.h"
#include "TTgmBot.h"

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
    static String dev_name;
    static TaskHandle_t h_task;
    static WiFiUDP ntp_udp;
    static NTPClient time_cli;
    static TTgmBot *pTgmBot;

    static void task(void *p);

    friend class Conf::TConf;
    friend class MedSession::TMedSession;

  public:
    TWiFiStuff(String dev_name, TConf *p_conf, TgmBot::TCbChangeFunction cb_change_formula);
    ~TWiFiStuff();

    static void tgb_send(const String& m, bool isMarkdownEnabled = true);
};
}
