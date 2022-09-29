#pragma once
#include <Arduino.h>
#include "TPrefs.h"
#include "TElementsDB.h"
#include "TTgmBot.h"

namespace WiFiStuff
{
using namespace Prefs;
using namespace ElementsDB;
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
    TWiFiStuff(String dev_name, TPrefs *p_prefs, TElementsDB *p_fdb);
    ~TWiFiStuff();
};
}
