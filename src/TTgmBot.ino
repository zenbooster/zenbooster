#include <functional> 
#include <cctype>
#include <locale>
#include <WiFi.h>
#include <tcpip_adapter.h>
#include "TTgmBot.h"
#include ".\\tg_certificate.h"
#include "TUtil.h"
#include "TCalcFormula.h"

namespace TgmBot
{
using namespace CalcFormula;
using namespace Util;

int TTgmBot::ref_cnt = 0;

void TTgmBot::show_help(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, 
    "*Команды общего назначения*:\n"
    "*help* \\- помощь\n"
    "*info* \\- информация о состоянии\n"
    "*sysinfo* \\- системная информация\n"
    "*reset* \\- перезагрузка\n"
    "*shutdown* \\- выключение\n\n"
    "*Команды для работы с базой формул*:\n"
    "*f\\_assign \\<имя\\> \\<текст\\>* \\- присвоить формуле с именем *\\<имя\\>* текст *\\<текст\\>*\n"
    "*f\\_assign \\<имя\\>* \\- удалить формулу с именем *\\<имя\\>*\n"
    "\\(*\\<имя\\>* должно иметь длину не больше 15 символов\\)\n"
    "*f\\_list* \\- показать список всех формул\n"
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

String ip2str(ip4_addr_t& ip)
{
  char buf[IP4ADDR_STRLEN_MAX];
  sprintf(buf, IPSTR, IP2STR(&ip));
  return buf;
}

void TTgmBot::show_sysinfo(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  
  String s_idf_ver = esp_get_idf_version();
  s_idf_ver.replace(".", "\\.");
  s_idf_ver.replace("-", "\\-");

  tcpip_adapter_ip_info_t ipInfo;
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
  String s_adapter_ip = TgmBot::ip2str(ipInfo.ip);
  s_adapter_ip.replace(".", "\\.");
  String s_adapter_gw = TgmBot::ip2str(ipInfo.gw);
  s_adapter_gw.replace(".", "\\.");

  pbot->sendMessage(msg, (
    String("*Система*:\n")+
    "*версия IDF*: " + s_idf_ver + "\n"
    "*MAC адрес адаптера*: " + WiFi.macAddress() + "\n"
    "*IP адрес адаптера*: " + s_adapter_ip + "\n"
    "*IP адрес шлюза*: " + s_adapter_gw + "\n"
    "*reset\\_reason*: " + esp_reset_reason() + "\n"
    "*размер свободной кучи*: " + heap_caps_get_free_size(MALLOC_CAP_8BIT) + "\n"
    "*максимальный размер свободного блока в куче*: " + heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) + "\n"
    ).c_str());
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
        String text = msg.text;
        //text = trim(text);
        text.trim();
        transform(text.begin(), text.end(), text.begin(), ::tolower);

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
          if(text == "sysinfo")
          {
            show_sysinfo(msg);
            break;
          }
          else
        #ifdef PIN_BATTARY
          if(text == "charge")
          {
            msg.isMarkdownEnabled = true;
            String s = TUtil::screen_mark_down(
              "Напряжение: " + String(battery.getBatteryVolts()) + " в.\n"
              "Уровень заряда: " + String(battery.getBatteryChargeLevel()) + "%\n"
              "Уровень заряда (по таблице): " + battery.getBatteryChargeLevel(true) + "%\n");

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
            esp_restart();
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
            char *s_cmd;
            bool is_no_args;

            if(s_cmd = "f_assign", is_no_args = (text == s_cmd), (is_no_args || text.startsWith(String(s_cmd) + " ")))
            {
              if(is_no_args)
              {
                pbot->sendMessage(msg, "Ошибка: требуется указать параметры!");
                break;
              }
              bool is_ok = false;
              String e_desc;
              String args = text.substring(strlen(s_cmd));
              args.trim();
              int pos = args.indexOf(' ');
              if(pos != -1)
              {
                String key = args.substring(0, pos);
                String val = args.substring(pos + 1);
                key.trim();
                val.trim();

                bool is_has_value = p_fdb->has_value(key);
                pbot->sendMessage(msg, String(is_has_value ? "Измен" : "Добавл") + "ение " + key + " = " + val);

                TCalcFormula *pcf;
                try
                {
                  pcf = new TCalcFormula(val);
                  is_ok = true;
                }
                catch(String e)
                {
                  e_desc = e;
                }

                if(is_ok) // Если формула не имеет ошибок
                {
                  is_ok = p_fdb->assign(key, val); // добавляем её в базу (или изменяем уже имеющуюся).

                  if(is_has_value) // Если изменили существующую формулу
                  {
                    if((*p_prefs)["f"] == key) // и она выбрана как текущая
                    {
                      p_prefs->reinit_value(key); // перекомпилируем её, чтобы изменения вступили в силу.
                    }
                  }
                }
              }
              else
              {
                pbot->sendMessage(msg, String("Удаление ") + args);
                is_ok = p_fdb->assign(args);
              }
              pbot->sendMessage(msg, String(is_ok ? "Ok" : "Ошибка") + String(e_desc.isEmpty() ? "!" : ": " + e_desc));
              break;
            }
            else
            if(text == "f_list")
            {
              msg.isMarkdownEnabled = true;
              String s_list = p_fdb->list();
              pbot->sendMessage(msg, String("*Список формул*:\n") + (s_list.length() ? s_list : "\\(пусто\\)"));
              break;
            }
            else
            {
              pbot->sendMessage(msg, "Ошибка синтаксиса!");
              show_help(msg);
              break;
            }
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

        //string opt = text.substr(0, opt_len);
        String opt = text.substring(0, opt_len);
        opt.trim();

        if(!p_prefs->contains(opt))
        {
          pbot->sendMessage(msg, (opt + " не является опцией!").c_str());
          break;
        }

        if(is_get)
        {
          pbot->sendMessage(msg, (opt + " = " + (*p_prefs)[opt]).c_str());
        }
        else
        {
          // Здесь можно завести колбэки OnSetValueBegin / OnSetValueEnd, чтобы вызывать функции таймера в них...
        #ifdef SOUND_DAC
          timer_pause(TIMER_GROUP_0, TIMER_0); // без этого уходит в перезагрузку при вызове dac_output_voltage из обработчика таймера
        #endif
          //string value = text.substr(pos_set+1);
          String value = text.substring(pos_set + 1);
          {
            value.trim();
            int res = p_prefs->set_value(opt, value);
          #ifdef SOUND_DAC
            timer_start(TIMER_GROUP_0, TIMER_0);
          #endif
            pbot->sendMessage(msg, String(res ? "Ok" : "Ошибка") + "!");
          }
        }
        break;
      }

      default:
        break;
    }
  }
}

TTgmBot::TTgmBot(String dev_name, TPrefs *p_prefs, TElementsDB *p_fdb):
  dev_name(dev_name),
  p_prefs(p_prefs),
  p_fdb(p_fdb)
#ifdef PIN_BATTARY
  , battery(PIN_BATTARY)
#endif
{
  if(ref_cnt)
  {
    throw "Only one instance of TTgmBot allowed!";
  }
  ref_cnt++;

  pcli = new SSLClient(wfcli, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
  //pcli = new WiFiClientSecure();
  //pcli->setCACert(telegram_cert);
  pbot = new AsyncTelegram2(*pcli);
  pbot->setUpdateTime(mtbs);
  pbot->setTelegramToken(BOT_TOKEN);
  
  Serial.print("\nTest Telegram connection... ");
  pbot->begin() ? Serial.println("Ok!") : Serial.println("Ошибка!");

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
