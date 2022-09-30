#pragma once
#include <Arduino.h>
#include "TPrefs.h"
#include "TFormulaDB.h"
#include "TTgmBot.h"

namespace WiFiStuff
{
using namespace Prefs;
using namespace FormulaDB;
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
    TWiFiStuff(String dev_name, TPrefs *p_prefs, TFormulaDB *p_fdb, TgmBot::TCbChangeFunction cb_change_formula);
    ~TWiFiStuff();
};
}
