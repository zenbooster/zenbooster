#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
//#include <sstream>
#include "TTgmBot.h"

namespace TgmBot
{
int TTgmBot::ref_cnt = 0;

// trim from start
/*
static inline String &ltrim(String &s)
{

    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline String &rtrim(String &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline String &trim(String &s) {
    return ltrim(rtrim(s));
}*/

void TTgmBot::show_help()
{
  send_message_markdown("*Команды*:\n*help* \\- помощь\n*info* \\- информация о состоянии\n*reset* \\- перезагрузка\n*shutdown* \\- выключение\n"
  #ifdef PIN_BATTARY
    "*charge* \\- уровень заряда\n"
  #endif  
  );
  send_message_markdown("*Опции*:\n" + String(p_prefs->get_desc().c_str()));
  send_message_markdown("\nУстановить значение: *option\\=value*\nЗапросить значение: *option?*");
}

void TTgmBot::show_info()
{
  send_message_markdown("*Опции*:\n" + String(p_prefs->get_values().c_str()));
}

/*String ReplaceString(String subject, const String& search, const String& replace)
{
    size_t pos = 0;
    while((pos = subject.substring(pos).indexOf(search)) != String::npos)
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}*/

void TTgmBot::send_message_markdown(String s)
{
  pbot->sendMessage(String(CHAT_ID), s.c_str(), "Markdown");
}

void TTgmBot::handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = pbot->messages[i].chat_id;

    if(chat_id != String(CHAT_ID))
      continue;

    String text = pbot->messages[i].text;

    /*String from_name = pbot->messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";
    */

    text.trim();
    text.toLowerCase();
    //transform(text.begin(), text.end(), text.begin(), ::tolower);

    Serial.print("\nText message received: ");
    Serial.println(text.c_str());

    int opt_len;
    bool is_get = (text[text.length()-1] == '?');
    int pos_set = text.indexOf('=');
    bool is_set = pos_set != -1;

    if(!(is_get ^ is_set)) // если не опция:
    {
      if(text == "help" || text == "/start")
      {
        show_help();
      }
      else
      if(text == "info")
      {
        show_info();
      }
      else
    #ifdef PIN_BATTARY
      if(text == "charge")
      {
          msg.isMarkdownEnabled = true;
          /*
          std::oStringstream ss;
          ss  << "Напряжение: " << battery.getBatteryVolts() << " в.\n"
              << "Уровень заряда: " << battery.getBatteryChargeLevel() << "%\n"
              << "Уровень заряда \\(по таблице\\): " << battery.getBatteryChargeLevel(true) << "%\n";
          */
          //String s = ReplaceString(ss.str(), ".", "\\.");
          String s =
            "Напряжение: " + battery.getBatteryVolts() + " в.\n" +
            "Уровень заряда: " + battery.getBatteryChargeLevel() + "%\n" +
            "Уровень заряда \\(по таблице\\): " + battery.getBatteryChargeLevel(true) + "%\n";        

        s.replace(".", "\\.");
        send_message_markdown(s);
      }
      else
    #endif
      if(text == "reset")
      {
        send_message_markdown(dev_name + " будет перезагружен...");
        // Wait until bot synced with telegram to prevent cyclic reboot
        /*while (!pbot->noNewMessage())
        {
          Serial.print(".");
          Serial.flush();
          delay(50);
        }*/
        TMyApplication::is_soft_reset = true;
        esp_deep_sleep(0); // так хитро, чтоб не затёрлась is_soft_reset
      }
      else
      if(text == "shutdown")
      {
        send_message_markdown(dev_name + " будет выключен...");
        // Wait until bot synced with telegram to prevent cyclic reboot
        /*while (!pbot->noNewMessage())
        {
          Serial.print(".");
          Serial.flush();
          delay(50);
        }*/
        esp_deep_sleep_start();
      }
      else
      {
        send_message_markdown("Ошибка синтаксиса!");
        show_help();
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

    String opt = text.substring(0, opt_len);
    opt.trim();

    if(!p_prefs->contains(opt.c_str()))
    {
      send_message_markdown(opt + " не является опцией!");
    }

    if(is_get)
    {
      send_message_markdown(opt + " = " + String((*p_prefs)[opt.c_str()].c_str()));
    }
    else
    {
      // Здесь можно завести колбэки OnSetValueBegin / OnSetValueEnd, чтобы вызывать функции таймера в них...
    #ifdef SOUND_DAC
      timer_pause(TIMER_GROUP_0, TIMER_0); // без этого уходит в перезагрузку при вызове dac_output_voltage из обработчика таймера
    #endif
      String value = text.substring(pos_set+1);
      value.trim();
      p_prefs->set_value(opt.c_str(), value.c_str());
    #ifdef SOUND_DAC
      timer_start(TIMER_GROUP_0, TIMER_0);
    #endif
      send_message_markdown("Ok!");
    }
  } // for (int i = 0; i < numNewMessages; i++)
}

void TTgmBot::run(void)// *p)
{
  if (millis() - bot_lasttime > mtbs)
  {
    int numNewMessages = pbot->getUpdates(pbot->last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = pbot->getUpdates(pbot->last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

TTgmBot::TTgmBot(string dev_name, TPrefs *p_prefs):
  dev_name(dev_name.c_str()),
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

  pcli = new WiFiClientSecure();
  pcli->setCACert(TELEGRAM_CERTIFICATE_ROOT);
  pcli->setInsecure();
  pbot = new UniversalTelegramBot(BOT_TOKEN, *pcli);
    
  //Serial.print("\nTest Telegram connection... ");
  //pbot->begin() ? Serial.println("OK") : Serial.println("NOK");

  /*char welcome_msg[128];
  snprintf(welcome_msg, 128, "BOT @%s в сети!\nНабери \"help\" для справки.", pbot->getBotName());
  int32_t chat_id = CHAT_ID; // You can discover your own chat id, with "Json Dump Bot"

  pbot->sendTo(chat_id, welcome_msg, "Markdown");
  */
  send_message_markdown("BOT @" + pbot->name + "в сети!\nНабери \"help\" для справки.");
  }

TTgmBot::~TTgmBot()
{
  // сделать удаление задачи.
  delete pbot;
  delete pcli;
  --ref_cnt;
}
}
