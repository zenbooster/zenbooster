#pragma once
#include "TPrefs.h"
#include <SSLClient.h>
//#include <WiFiClientSecure.h>
#include ".\\AsyncTelegram2.h"
#ifdef PIN_BATTARY
# include <Battery18650Stats.h>
#endif

#define BOT_TOKEN "" // zenbooster_device
#define CHAT_ID 0

namespace TgmBot
{
using namespace Prefs;

class TTgmBot
{
  private:
    const unsigned long mtbs = 250; // mean time between scan messages
    string dev_name;
    TPrefs *p_prefs;
    WiFiClient wfcli;
    SSLClient *pcli;
    //WiFiClientSecure *pcli;
    AsyncTelegram2 *pbot;
  #ifdef PIN_BATTARY
    Battery18650Stats battery;
  #endif
    static int ref_cnt;

  public:
    TTgmBot(string dev_name, TPrefs *p_prefs);
    ~TTgmBot();

    void run(void);
    void show_help(TBMessage& msg);
    void show_info(TBMessage& msg);
};
}
