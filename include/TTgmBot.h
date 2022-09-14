#pragma once
#include "TPrefs.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
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
    const unsigned long mtbs = 1000;//250; // mean time between scan messages
    unsigned long bot_lasttime;
    String dev_name;
    TPrefs *p_prefs;
    WiFiClientSecure *pcli;
    UniversalTelegramBot *pbot;
  #ifdef PIN_BATTARY
    Battery18650Stats battery;
  #endif
    static int ref_cnt;

    void send_message_markdown(String s);
    void handleNewMessages(int numNewMessages);

  public:
    TTgmBot(string dev_name, TPrefs *p_prefs);
    ~TTgmBot();

    void run(void);
    void show_help();
    void show_info();
};
}
