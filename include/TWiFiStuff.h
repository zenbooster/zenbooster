#pragma once
#include "TPrefs.h"
#include "TWebSrv.h"
#include "TTgmBot.h"

namespace WiFiStuff
{
using namespace TgmBot;
using namespace WebSrv;

class TWiFiStuff
{
  private:
    static int ref_cnt;
    string dev_name;
    TTgmBot *pTgmBot;
    TWebSrv *pWebSrv;

    static void task(void *p);

  public:
    TWiFiStuff(string dev_name, TPrefs *p_prefs);
    ~TWiFiStuff();
};
}
