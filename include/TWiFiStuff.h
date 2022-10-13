#pragma once
#include <Arduino.h>
#include "TConf.h"
#include "TTgmBot.h"

namespace WiFiStuff
{
using namespace Conf;
using namespace TgmBot;

class TWiFiStuff
{
  private:
    static int ref_cnt;
    String dev_name;
    TaskHandle_t h_task;
    TTgmBot *pTgmBot;

    static void task(void *p);

  public:
    TWiFiStuff(String dev_name, TConf *p_conf, TgmBot::TCbChangeFunction cb_change_formula);
    ~TWiFiStuff();

    void tgb_send(const String& m, bool isMarkdownEnabled = true);
};
}
