#pragma once
#include "common.h"
#include "TSingleton.h"
#include "TCalcFormula.h"
#include <WiFiClient.h>
#include <SSLClient.h>
//#include <WiFiClientSecure.h>
#include ".\\AsyncTelegram2.h"
#ifdef PIN_BATTARY
# include <Battery18650Stats.h>
#endif
#include <functional>

namespace MyApplication {class TMyApplication;}

namespace TgmBot
{
using namespace Singleton;
using namespace CalcFormula;
using namespace std;

typedef function<void(TCalcFormula *)> TCbChangeFunction;

class TTgmBot: public TSingleton<TTgmBot>
{
  private:
    static String token;
    static uint64_t chat_id;
    //const unsigned long mtbs = 250; // mean time between scan messages
    const unsigned long mtbs = 500; // mean time between scan messages
    String dev_name;
    WiFiClient wfcli;
    SSLClient *pcli;
    //WiFiClientSecure *pcli;
    AsyncTelegram2 *pbot;
  #ifdef PIN_BATTARY
    Battery18650Stats battery;
  #endif
    TCbChangeFunction cb_change_formula;
    QueueHandle_t queue;

    const char *get_class_name();

    void show_help(TBMessage& msg);
    void show_info(TBMessage& msg);
    void show_sysinfo(TBMessage& msg);
    void send_config(TBMessage& msg);

    void flush_message(void);
    bool ProcessQueue(void);

    bool cmd_conf_1_arg(const String& s_cmd, const String& text, void (*p_mtd)(const DynamicJsonDocument& ), TBMessage& msg);

    friend class MyApplication::TMyApplication;

  public:
    TTgmBot(String dev_name, TCbChangeFunction cb_change_formula);
    ~TTgmBot();

    void run(void);
    // isMarkdownEnabled - ещё не реализовано...
    void send(const char *m, bool isMarkdownEnabled = true);
    inline void send(const std::shared_ptr<char> m, bool isMarkdownEnabled = true)
    {
      send(m.get(), isMarkdownEnabled);
    }

    inline void send(const String& m, bool isMarkdownEnabled = true)
    {
      send(m.c_str(), isMarkdownEnabled);
    }

    void say_goodbye(void);
};
}
