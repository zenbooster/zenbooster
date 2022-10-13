#pragma once
#include "common.h"
#include "TConf.h"
#include "TCalcFormula.h"
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
using namespace Conf;
using namespace CalcFormula;

typedef function<void(TCalcFormula *)> TCbChangeFunction;

class TTgmBot
{
  private:
    //const unsigned long mtbs = 250; // mean time between scan messages
    const unsigned long mtbs = 500; // mean time between scan messages
    String dev_name;
    TConf *p_conf;
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
    void send_config(TBMessage& msg);

    void flush_message(void);

    bool cmd_conf_1_arg(const String& s_cmd, const String& text, void (TConf::*p_mtd)(const DynamicJsonDocument& ), TBMessage& msg);

  public:
    TTgmBot(String dev_name, TConf *p_conf, TCbChangeFunction cb_change_formula);
    ~TTgmBot();

    void run(void);
    void send(const String& m, bool isMarkdownEnabled = true);
};
}
