#pragma once
#include "TPrefs.h"
#include <SSLClient.h>
//#include <WiFiClientSecure.h>
#include ".\\AsyncTelegram2.h"

#define BOT_TOKEN "5430273296:AAEHqXHPXFacLsDAmFIKzv9trh--m5vMWJI" // zenbooster_nfm_bot
#define CHAT_ID 443232875

namespace TgmBot
{
using namespace Prefs;

class TTgmBot
{
  private:
    const unsigned long mtbs = 1000;//250; // mean time between scan messages
    string dev_name;
    TPrefs *p_prefs;
    WiFiClient wfcli;
    SSLClient *pcli;
    //WiFiClientSecure *pcli;
    AsyncTelegram2 *pbot;
    static int ref_cnt;

  public:
    TTgmBot(string dev_name, TPrefs *p_prefs);
    ~TTgmBot();

    void run(void);
    void show_help(TBMessage& msg);
    void show_info(TBMessage& msg);
};
}
