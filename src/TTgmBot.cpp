#include <functional> 
#include <cctype>
#include <locale>
#include <memory>
#include "TTgmBot.h"
#include ".\\tg_certificate.h"
#include "TUtil.h"
#include "TMyApplication.h"
#include "TWorker/TWorker.h"
#include "TConf.h"
#include "ArduinoJson.h"

namespace TgmBot
{
using namespace Util;
using namespace MyApplication;
using namespace Worker;
using namespace Conf;

String TTgmBot::token = "";
uint64_t TTgmBot::chat_id = 0;

const char *TTgmBot::get_class_name()
{
  return "TTgmBot";
}

void TTgmBot::show_help(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg,
    "*Команды общего назначения*:\n"
    "*help* \\- помощь\n"
    "*info* \\- информация о состоянии\n"
    "*sysinfo* \\- системная информация\n"
    "*reset* \\- перезагрузка\n"
    "*shutdown* \\- выключение\n"
  #ifdef PIN_BATTARY
    "*charge* \\- уровень заряда\n"
  #endif
  );
  pbot->sendMessage(msg,
    "*Команды для работы с конфигурационным JSON*:\n"
    "*getconf* \\- отправить JSON с настройками\n"
    "*vdtconf \\<JSON\\>* \\- проверить JSON с настройками\n"
    "*addconf \\<JSON\\>* \\- применить JSON с настройками, не очищать базу формул\n"
    "*setconf \\<JSON\\>* \\- применить JSON с настройками, предварительно очистить базу формул\n"
  );
  flush_message();
  pbot->sendMessage(msg,
    "*Команды для работы с базой формул*:\n"
    "*fassign \\<имя\\> \\<текст\\>* \\- присвоить формуле с именем *\\<имя\\>* текст *\\<текст\\>*\n"
    "*fassign \\<имя\\>* \\- удалить формулу с именем *\\<имя\\>*\n"
    "\\(*\\<имя\\>* должно иметь длину не больше 15 символов\\)\n"
    "*flist* \\- показать список всех формул\n"
  );
  pbot->sendMessage(msg, ("*Опции*:\n" + TConf::get_prefs()->get_desc()).c_str());
  flush_message();
  pbot->sendMessage(msg, "\nУстановить значение: *option\\=value*\nЗапросить значение: *option?*");
}

void TTgmBot::show_info(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, ("*Опции*:\n" + TConf::get_prefs()->get_values()).c_str());
}

void TTgmBot::show_sysinfo(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, TWorker::cmd_sysinfo().get());
}

void TTgmBot::send_config(TBMessage& msg)
{
  msg.isMarkdownEnabled = true;
  pbot->sendMessage(msg, TWorker::cmd_getconf().get());
}

void TTgmBot::flush_message(void)
{
    while (!pbot->noNewMessage())
    {
      delay(50);
    }
}

bool TTgmBot::cmd_conf_1_arg(const String& s_cmd, const String& text, void (*p_mtd)(const DynamicJsonDocument& ), TBMessage& msg)
{
  bool is_no_args = (text == s_cmd);
  bool res = (is_no_args || text.startsWith(s_cmd + " "));

  do // fake loop
  {
    if(res)
    {
      if(is_no_args)
      {
        pbot->sendMessage(msg, "Ошибка: требуется указать параметр!");
        break;
      }

      String e_desc;
      String args = text.substring(s_cmd.length());
      args.trim();
      
      try
      {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, args.c_str());
        p_mtd(doc);
      }
      catch(String& e)
      {
        e_desc = e;
      }
      pbot->sendMessage(msg, String(e_desc.isEmpty() ? "Ok" : "Ошибка: ") + e_desc + "!");
      break;
    }
  } while(false);

  return res;
}

bool TTgmBot::ProcessQueue(void)
{
  bool res;
  TBMessage msg;
  char *p = NULL;

  res = xQueueReceive(queue, &p, 0);
  if(res)
  {
    msg.chatId = chat_id;
    msg.isMarkdownEnabled = true;

    // Как выяснилось, если вернёт false, сообщение всё равно может отправиться,
    // по этому, при плохой связи, может произойти двойная отправка. Что бы этого
    // не происходило, если ProcessQueue вернёт true, надо будет вызвать flush_message.
    for(; !pbot->sendMessage(msg, p, nullptr, true);)
    {
      vTaskDelay(250);
    }

    delete p;
    msg.isMarkdownEnabled = false;
  }
  return res;
}

void TTgmBot::run(void)
{
  // a variable to store telegram message data
  TPrefs *p_prefs = TConf::get_prefs();
  TFormulaDB *p_fdb = TConf::get_fdb();
  TBMessage msg;
  bool is_flush = false;

  // Пока очередь не пуста:
  for(; ProcessQueue();)
  {
    is_flush = true;
  }

  // if there is an incoming message...
  if (pbot->getNewMessage(msg))
  {
    // check what kind of message I received
    MessageType msgType = msg.messageType;

    switch (msgType)
    {
      case MessageText:
      {
        // received a text message
        String& text = msg.text;

        text.trim();
        transform(text.begin(), text.end(), text.begin(), ::tolower);

        TWorker::printf("Получено текстовое сообщение: %s\n", text.c_str());

        int opt_len;
        bool is_get = (text[text.length()-1] == '?');
        int pos_set = text.indexOf('=');
        bool is_set = pos_set != -1;

        const char *s_cmd;
        bool is_no_args;

        do // fake loop
        {
          if(!(is_get ^ is_set)) // если не опция:
          { // команды общего назначения:
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
              flush_message();
              TSleepMode::reset();
              break;
            }
            else
            if(text == "shutdown")
            {
              pbot->sendMessage(msg, (dev_name + " будет выключен...").c_str());
              // Wait until bot synced with telegram to prevent cyclic reboot
              flush_message();
              TSleepMode::shutdown();
              break;
            }
            else // команды для работы с конфигурационным JSON:
            if(text == "getconf")
            {
              send_config(msg);
              break;
            }
            else
            if(cmd_conf_1_arg("vdtconf", text, &TConf::validate_json, msg))
            {
              break;
            }
            else
            if(cmd_conf_1_arg("addconf", text, &TConf::add_json, msg))
            {
              break;
            }
            else
            if(cmd_conf_1_arg("setconf", text, &TConf::set_json, msg))
            {
              break;
            }
            else // команды для работы с базой формул:
            if(s_cmd = "fassign", is_no_args = (text == s_cmd), (is_no_args || text.startsWith(String(s_cmd) + " ")))
            {
              if(is_no_args)
              {
                pbot->sendMessage(msg, "Ошибка: требуется указать параметры!");
                break;
              }

              String e_desc;
              String args = text.substring(strlen(s_cmd));
              args.trim();

              int pos = args.indexOf(' ');
              bool is_add_or_ed = pos != -1;

              String key = is_add_or_ed ? args.substring(0, pos) : args;
              key.trim();
              bool is_has_value;
              try
              {
                is_has_value = p_fdb->has_value(key);
              }
              catch(const String& e)
              {
                String m = "Ошибка: " + e + "!";
                pbot->sendMessage(msg, m);
                break;
              }

              if(is_add_or_ed)
              {
                String val = args.substring(pos + 1);
                val.trim();

                pbot->sendMessage(msg, String(is_has_value ? "Измен" : "Добавл") + "ение " + key + " = " + val);

                {
                  TCalcFormula *pcf;
                  try
                  {
                    pcf = TCalcFormula::compile(val);

                    p_fdb->assign(key, val); // добавляем её в базу (или изменяем уже имеющуюся).

                    if(is_has_value && ((*p_prefs)["f"] == key)) // Если изменили существующую формулу и она выбрана как текущая
                    {
                      cb_change_formula(pcf);
                    }
                    else
                    {
                      delete pcf;
                    }
                  }
                  catch(String& e)
                  {
                    e_desc = e;
                  }
                }
              }
              else
              {
                pbot->sendMessage(msg, String("Удаление ") + args);
                if(is_has_value && ((*p_prefs)["f"] == key)) // Если удаляем существующую формулу и она выбрана как текущая
                {
                  e_desc = "нельзя удалить формулу, которая выбрана как текущая";
                }
                else
                {
                  try
                  {
                    p_fdb->assign(args);
                  }
                  catch(const String& e)
                  {
                    e_desc = e;
                  }
                }
              }
              pbot->sendMessage(msg, String(e_desc.isEmpty() ? "Ok" : "Ошибка: ") + e_desc + "!");
              break;
            } // f_assign
            else
            if(text == "flist")
            {
              msg.isMarkdownEnabled = true;
              String sck = (*p_prefs)["f"]; // ключ - имя текущей формулы
              String s_list = p_fdb->list(&sck);
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
            String value = text.substring(pos_set + 1);
            {
              value.trim();
              try
              {
            // Здесь можно завести колбэки OnSetValueBegin / OnSetValueEnd, чтобы вызывать функции таймера в них...
            #ifdef SOUND_DAC
              timer_pause(TIMER_GROUP_0, TIMER_0); // без этого уходит в перезагрузку при вызове dac_output_voltage из обработчика таймера
            #endif
                p_prefs->set_value(opt, value);
            #ifdef SOUND_DAC
              timer_start(TIMER_GROUP_0, TIMER_0);
            #endif
                send("Ok\\!");
              }
              catch(String& e)
              {
            #ifdef SOUND_DAC
              timer_start(TIMER_GROUP_0, TIMER_0);
            #endif
                send(TUtil::screen_mark_down(("Ошибка: " + e +  "!")));
              }
            }
          }
        } while(false);

        is_flush = true;
        break;
      } // case MessageText:

      default:
        break;
    }
  }

  if(is_flush)
  {
    flush_message();
  }
}

void TTgmBot::say_goodbye(void)
{
  send(TWorker::screen_mark_down(TWorker::sprintf("Бот @%s выключился!", pbot->getBotName())));
}

TTgmBot::TTgmBot(String dev_name, TCbChangeFunction cb_change_formula):
  dev_name(dev_name)
#ifdef PIN_BATTARY
  , battery(PIN_BATTARY)
#endif
  , cb_change_formula(cb_change_formula)
{
  pcli = new SSLClient(wfcli, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
  //pcli = new WiFiClientSecure();
  //pcli->setCACert(telegram_cert);
  pbot = new AsyncTelegram2(*pcli);
  pbot->setUpdateTime(mtbs);
  pbot->setTelegramToken(token.c_str());
  
  TWorker::print("\nПроверяем Телеграм-соединение... ");
  pbot->begin() ? TWorker::println("Ok!") : TWorker::println("Ошибка!");

  queue = xQueueCreate(4, sizeof(char *));
  if (queue == NULL) {
    throw String("TTgmBot::TTgmBot(..): ошибка создания очереди");
  }

  pbot->sendTo(chat_id, TWorker::sprintf("Бот @%s в сети!\nНаберите \"help\" для справки.", pbot->getBotName()).get());
}

TTgmBot::~TTgmBot()
{
  if(queue)
  {
    vQueueDelete(queue);
  }

  if(pbot)
  {
    delete pbot;
  }
  if(pcli)
  {
    delete pcli;
  }
}

void TTgmBot::send(const char *m, bool isMarkdownEnabled)
{
  if(pbot)
  {
    char *p = new char[strlen(m) + 1];
    strcpy(p, m);
    xQueueSend(queue, &p, portMAX_DELAY);
  }
}
}
