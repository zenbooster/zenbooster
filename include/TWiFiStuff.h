#pragma once
#include <string>
#include "TPrefs.h"
#include "TTgmBot.h"

namespace WiFiStuff
{
using namespace std;
using namespace TgmBot;

class TWiFiStuff
{
  private:
    static int ref_cnt;
    string dev_name;
    TaskHandle_t h_task;
    TTgmBot *pTgmBot;

    static void task(void *p);

  public:
    TWiFiStuff(string dev_name, TPrefs *p_prefs);
    ~TWiFiStuff();
};
}
