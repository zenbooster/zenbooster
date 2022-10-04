#pragma once
#include "common.h"
#include "TPrefs.h"
#include "TFormulaDB.h"
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
using namespace FormulaDB;

typedef function<void(TCalcFormula *)> TCbChangeFunction;

class TTgmBot
{
  private:
    //const unsigned long mtbs = 250; // mean time between scan messages
    const unsigned long mtbs = 500; // mean time between scan messages
    String dev_name;
    TPrefs *p_prefs;
    TFormulaDB *p_fdb;
    WiFiClient wfcli;
    SSLClient *pcli;
    //WiFiClientSecure *pcli;
    AsyncTelegram2 *pbot;
  #ifdef PIN_BATTARY
    Battery18650Stats battery;
  #endif
    static int ref_cnt;
    TCbChangeFunction cb_change_formula;
    QueueHandle_t queue;

    void show_help(TBMessage& msg);
    void show_info(TBMessage& msg);
    void show_sysinfo(TBMessage& msg);
  public:
    TTgmBot(String dev_name, TPrefs *p_prefs, TFormulaDB *p_fdb, TCbChangeFunction cb_change_formula);
    ~TTgmBot();

    void run(void);
    void send(const String& m, bool isMarkdownEnabled = true);
};
}
