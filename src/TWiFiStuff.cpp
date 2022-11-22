#include "common.h"
#include "TWiFiStuff.h"
#include "TConf.h"
#include "TMqttClient.h"

namespace WiFiStuff
{
using namespace MyApplication;

TTask *TWiFiStuff::p_task = NULL;
SemaphoreHandle_t TWiFiStuff::xDtorMutex = NULL;
TaskHandle_t TWiFiStuff::h_dtor_task = NULL;
WiFiUDP TWiFiStuff::ntp_udp;
NTPClient TWiFiStuff::time_cli(ntp_udp);
TTgmBot *TWiFiStuff::pTgmBot = NULL;
bool TWiFiStuff::is_mqtt = false;
TMQTTClient *TWiFiStuff::p_mqtt = NULL;

const char *TWiFiStuff::get_class_name()
{
    return "TWiFiStuff";
}

void TWiFiStuff::task(void *p)
{
  static unsigned long old_sec = getEpochTime();
  for(;;)
  {
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    time_cli.update();
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    if (pTgmBot)
      pTgmBot->run();
    
    if(p_mqtt)
    {
      unsigned long sec = getEpochTime();
      if(sec - old_sec >= 3)
      {
        p_mqtt->run();
        old_sec = sec;
      }
    }

    xSemaphoreTake(xDtorMutex, portMAX_DELAY);
    if(h_dtor_task)
    {
      xTaskNotify(h_dtor_task, 0, eNoAction);
    }
    xSemaphoreGive(xDtorMutex);

    yield();
  }
}

TWiFiStuff::TWiFiStuff(String dev_name, TgmBot::TCbChangeFunction cb_change_formula)
{
  time_cli.begin();

  pTgmBot = new TTgmBot(dev_name, cb_change_formula);

  time_cli.update();
  p_mqtt = new TMQTTClient();

  xDtorMutex = xSemaphoreCreateMutex();
  p_task = new TTask(task, "TWiFiStuff", TWIFISTUFF_TASK_STACK_SIZE, this, tskIDLE_PRIORITY + 2, portNUM_PROCESSORS - 2);
}

TWiFiStuff::~TWiFiStuff()
{
  pTgmBot->say_goodbye(); // попрощаемся
  // Нам нужно уведомить себя о том, что цикл в задаче совершил очередную
  // итерацию, чтобы гарантировать отправку прощального сообщения:
  xSemaphoreTake(xDtorMutex, portMAX_DELAY);
  // сообщаем, что уведомить надо нас:
  h_dtor_task = xTaskGetCurrentTaskHandle();
  xSemaphoreGive(xDtorMutex);
  // Чтоб гарантировать полную итерацию, вызываем два раза:
  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

  // Мы всё сделали для того, чтобы сообщение ушло. Теперь можно удалить задачу:
  if(p_task)
  {
    delete p_task;
  }

  vSemaphoreDelete(xDtorMutex);

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
  return is_mqtt;// && p_mqtt->is_connected();
}

void TWiFiStuff::mqtt_send(const char *topic, const DynamicJsonDocument *p)
{
  p_mqtt->send(topic, p);
}
}
