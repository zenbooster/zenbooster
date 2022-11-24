#include "common.h"
#include "TWiFiStuff.h"
#include "TConf.h"
#include "TMqttClient.h"
#include "TMyApplication.h"
#include <StreamString.h>

namespace WiFiStuff
{
using namespace MyApplication;

TTask *TWiFiStuff::p_task = NULL;
SemaphoreHandle_t TWiFiStuff::x_dtor_mutex = NULL;
TaskHandle_t TWiFiStuff::h_dtor_task = NULL;
WiFiUDP TWiFiStuff::ntp_udp;
NTPClient TWiFiStuff::time_cli(ntp_udp);
TTgmBot *TWiFiStuff::pTgmBot = NULL;
bool TWiFiStuff::is_mqtt = false;
TMQTTClient *TWiFiStuff::p_mqtt = NULL;
SemaphoreHandle_t TWiFiStuff::x_mqtt_send_mutex;
bool TWiFiStuff::is_mqtt_disabling = false;

const char *TWiFiStuff::get_class_name()
{
    return "TWiFiStuff";
}

void TWiFiStuff::task(void *p)
{
  for(;;)
  {
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    time_cli.update();
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    if (pTgmBot)
      pTgmBot->run();
    
    // если пользак бота сменит опцию mqtt, при
    // необходимости создадим или уничтожим объект:
    set_mqtt_active(is_mqtt);
    
    if(p_mqtt)
    {
      p_mqtt->run();
    }

    xSemaphoreTake(x_dtor_mutex, portMAX_DELAY);
    if(h_dtor_task)
    {
      xTaskNotify(h_dtor_task, 0, eNoAction);
    }
    xSemaphoreGive(x_dtor_mutex);

    yield();
  }
}

TWiFiStuff::TWiFiStuff(String dev_name, TgmBot::TCbChangeFunction cb_change_formula)
{
  time_cli.begin();

  pTgmBot = new TTgmBot(dev_name, cb_change_formula);

  time_cli.update();
  //x_mqtt_send_mutex = xSemaphoreCreateMutex();
  //p_mqtt = new TMQTTClient();
  set_mqtt_active(true);
  x_dtor_mutex = xSemaphoreCreateMutex();
  p_task = new TTask(task, "TWiFiStuff", TWIFISTUFF_TASK_STACK_SIZE, this, tskIDLE_PRIORITY + 2, portNUM_PROCESSORS - 2);
}

void TWiFiStuff::wait_for_send()
{
  // Нам нужно уведомить себя о том, что цикл в задаче совершил очередную
  // итерацию, чтобы гарантировать отправку прощального сообщения:
  xSemaphoreTake(x_dtor_mutex, portMAX_DELAY);
  // сообщаем, что уведомить надо нас:
  h_dtor_task = xTaskGetCurrentTaskHandle();
  xSemaphoreGive(x_dtor_mutex);
  // Чтоб гарантировать полную итерацию, вызываем два раза:
  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
}

TWiFiStuff::~TWiFiStuff()
{
  set_mqtt_active(false);

  pTgmBot->say_goodbye(); // попрощаемся
  wait_for_send();

  // Мы всё сделали для того, чтобы сообщение ушло. Теперь можно удалить задачу:
  if(p_task)
  {
    delete p_task;
  }
  vSemaphoreDelete(x_dtor_mutex);

  if(pTgmBot)
  {
    delete pTgmBot;
  }
}

void TWiFiStuff::tgb_send(const char *m, bool isMarkdownEnabled)
{
  if(pTgmBot)
  {
    pTgmBot->send(m, isMarkdownEnabled);
  }
}

unsigned long TWiFiStuff::getEpochTime()
{
  return time_cli.getEpochTime();
}

bool TWiFiStuff::is_mqtt_active()
{
  return is_mqtt;
}

void TWiFiStuff::set_mqtt_active(bool is)
{
  if(is_mqtt_disabling)
  {
    return;
  }
  
  is_mqtt_disabling = true;

  if(is)
  {
    if(!p_mqtt)
    {
      x_mqtt_send_mutex = xSemaphoreCreateMutex();
      p_mqtt = new TMQTTClient();

      if(TWiFiStuff::is_mqtt_active())
      {
          DynamicJsonDocument doc = TConf::get_json(); //TMyApplication::p_conf->get_json();

          doc["when"] = TWiFiStuff::getEpochTime();
          TWiFiStuff::mqtt_send("hello", &doc);
      }
    }
  }
  else
  {
    if(p_mqtt)
    {
      {
          DynamicJsonDocument doc(64);
          doc["when"] = TWiFiStuff::getEpochTime();
          TWiFiStuff::mqtt_send("bye", &doc);
      }
      wait_for_send();

      TMQTTClient *p = p_mqtt;
      p_mqtt = NULL;
      xSemaphoreTake(x_mqtt_send_mutex, portMAX_DELAY);
      delete p;
      xSemaphoreGive(x_mqtt_send_mutex);
      vSemaphoreDelete(x_mqtt_send_mutex);

      // такое бывает, только когда нас вызвали из деструктора:
      if(is_mqtt)
      {
        is_mqtt = false;
      }
    }
  }
  is_mqtt_disabling = false;
}

void TWiFiStuff::mqtt_send(const char *topic, const DynamicJsonDocument *p)
{
  if(p_mqtt)
  {
    xSemaphoreTake(x_mqtt_send_mutex, portMAX_DELAY);
    p_mqtt->send(topic, p);
    xSemaphoreGive(x_mqtt_send_mutex);
  }
}
}
