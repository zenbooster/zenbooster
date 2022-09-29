#pragma once
#include "common.h"
#include "TPrefs.h"
#include "TElementsDB.h"
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
using namespace ElementsDB;

class TTgmBot
{
  private:
    const unsigned long mtbs = 250; // mean time between scan messages
    String dev_name;
    TPrefs *p_prefs;
    TElementsDB *p_fdb;
    WiFiClient wfcli;
    SSLClient *pcli;
    //WiFiClientSecure *pcli;
    AsyncTelegram2 *pbot;
  #ifdef PIN_BATTARY
    Battery18650Stats battery;
  #endif
    static int ref_cnt;

    void show_help(TBMessage& msg);
    void show_info(TBMessage& msg);
    void show_sysinfo(TBMessage& msg);
  public:
    TTgmBot(String dev_name, TPrefs *p_prefs, TElementsDB *p_fdb);
    ~TTgmBot();

    void run(void);
};
}
