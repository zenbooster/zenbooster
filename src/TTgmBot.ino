#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <sstream>
#include "TTgmBot.h"
#include ".\\tg_certificate.h"

namespace TgmBot
{
int TTgmBot::ref_cnt = 0;

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

void TTgmBot::show_help(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, "*Команды*:\n*help* \\- помощь\n*info* \\- информация о состоянии\n*reset* \\- перезагрузка\n*shutdown* \\- выключение\n"
  #ifdef PIN_BATTARY
    "*charge* \\- уровень заряда\n"
  #endif  
  );
  pbot->sendMessage(msg, ("*Опции*:\n" + p_prefs->get_desc()).c_str());
  pbot->sendMessage(msg, "\nУстановить значение: *option\\=value*\nЗапросить значение: *option?*");
}

void TTgmBot::show_info(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, ("*Опции*:\n" + p_prefs->get_values()).c_str());
}

std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void TTgmBot::run(void)// *p)
{
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (pbot->getNewMessage(msg))
  {
    // check what kind of message I received
    String tgReply;
    MessageType msgType = msg.messageType;

    switch (msgType)
    {
      case MessageText:
      {
        // received a text message
        string text = msg.text.c_str();
        text = trim(text);
        transform(text.begin(), text.end(), text.begin(), ::tolower);

        Serial.print("\nText message received: ");
        Serial.println(text.c_str());

        int opt_len;
        bool is_get = (text[text.length()-1] == '?');
        int pos_set = text.find('=');
        bool is_set = pos_set != string::npos;

        if(!(is_get ^ is_set)) // если не опция:
        {
          if(text == "help" || text == "/start")
          {
            show_help(msg);
            break;
          }
          else
          if(text == "info")
          {
            show_info(msg);
            break;
          }
          else
        #ifdef PIN_BATTARY
          if(text == "charge")
          {
              msg.isMarkdownEnabled = true;
              std::ostringstream ss;
              ss  << "Напряжение: " << battery.getBatteryVolts() << " в.\n"
                  << "Уровень заряда: " << battery.getBatteryChargeLevel() << "%\n"
                  << "Уровень заряда \\(по таблице\\): " << battery.getBatteryChargeLevel(true) << "%\n";
              string s = ReplaceString(ss.str(), ".", "\\.");

            pbot->sendMessage(msg, s.c_str());
            break;
          }
          else
        #endif
          if(text == "reset")
          {
            pbot->sendMessage(msg, (dev_name + " будет перезагружен...").c_str());
            // Wait until bot synced with telegram to prevent cyclic reboot
            while (!pbot->noNewMessage())
            {
              Serial.print(".");
              Serial.flush();
              delay(50);
            }
            ESP.restart();
          }
          else
          if(text == "shutdown")
          {
            pbot->sendMessage(msg, (dev_name + " будет выключен...").c_str());
            // Wait until bot synced with telegram to prevent cyclic reboot
            while (!pbot->noNewMessage())
            {
              Serial.print(".");
              Serial.flush();
              delay(50);
            }
            esp_deep_sleep_start();
          }
          else
          {
            pbot->sendMessage(msg, "Ошибка синтаксиса!");
            show_help(msg);
            break;
          }
        }

        if(is_get)
        {
          opt_len = text.length()-1;
        }
        else
        {
          opt_len = pos_set;
        }

        string opt = text.substr(0, opt_len);
        opt = trim(opt);

        if(!p_prefs->contains(opt))
        {
          pbot->sendMessage(msg, (opt + " не является опцией!").c_str());
          break;
        }

        if(is_get)
        {
          pbot->sendMessage(msg, (opt + " = " + string((*p_prefs)[opt])).c_str());
        }
        else
        {
          // Здесь можно завести колбэки OnSetValueBegin / OnSetValueEnd, чтобы вызывать функции таймера в них...
        #ifdef SOUND_DAC
          timer_pause(TIMER_GROUP_0, TIMER_0); // без этого уходит в перезагрузку при вызове dac_output_voltage из обработчика таймера
        #endif
          string value = text.substr(pos_set+1);
          p_prefs->set_value(opt, trim(value));
        #ifdef SOUND_DAC
          timer_start(TIMER_GROUP_0, TIMER_0);
        #endif
          pbot->sendMessage(msg, "Ok!");
        }
        break;
      }

      default:
        break;
    }
  }
}

TTgmBot::TTgmBot(string dev_name, TPrefs *p_prefs):
  dev_name(dev_name),
  p_prefs(p_prefs)
#ifdef PIN_BATTARY
  , battery(PIN_BATTARY)
#endif
{
  if(ref_cnt)
  {
    throw "Only one instance of TTgmBot allowed!";
  }
  ref_cnt++;

  pcli = new SSLClient(wfcli, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR );
  //pcli = new WiFiClientSecure();
  //pcli->setCACert(telegram_cert);
  pbot = new AsyncTelegram2(*pcli);
  pbot->setUpdateTime(mtbs);
  pbot->setTelegramToken(BOT_TOKEN);
  
  Serial.print("\nTest Telegram connection... ");
  pbot->begin() ? Serial.println("OK") : Serial.println("NOK");

  char welcome_msg[128];
  snprintf(welcome_msg, 128, "BOT @%s в сети!\nНабери \"help\" для справки.", pbot->getBotName());
  int32_t chat_id = CHAT_ID; // You can discover your own chat id, with "Json Dump Bot"

  pbot->sendTo(chat_id, welcome_msg);
}

TTgmBot::~TTgmBot()
{
  // сделать удаление задачи.
  delete pbot;
  delete pcli;
  --ref_cnt;
}
}
