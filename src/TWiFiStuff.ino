#include "TWiFiStuff.h"
#include "TMyApplication.h"

namespace WiFiStuff
{
using namespace MyApplication;

int TWiFiStuff::ref_cnt = 0;
//String TWiFiStuff::dev_name;
TaskHandle_t TWiFiStuff::h_task = NULL;
SemaphoreHandle_t TWiFiStuff::xDtorMutex = NULL;
TaskHandle_t TWiFiStuff::h_dtor_task = NULL;
WiFiUDP TWiFiStuff::ntp_udp;
NTPClient TWiFiStuff::time_cli(ntp_udp);
TTgmBot *TWiFiStuff::pTgmBot = NULL;

void TWiFiStuff::task(void *p)
{
  //TWiFiStuff *pthis = static_cast<TWiFiStuff *>(p);
  //TTgmBot *pTgmBot = TWiFiStuff::pTgmBot;

  for(;;)
  {
    xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
    time_cli.update();
    xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);

    if (pTgmBot)
      pTgmBot->run();

    xSemaphoreTake(xDtorMutex, portMAX_DELAY);
    if(h_dtor_task)
    {
      xTaskNotify(h_dtor_task, 0, eNoAction);
    }
    xSemaphoreGive(xDtorMutex);

    yield();
  }
}

TWiFiStuff::TWiFiStuff(String dev_name, TConf *p_conf, TgmBot::TCbChangeFunction cb_change_formula)
{
  if(ref_cnt)
  {
    throw "Only one instance of TWiFiStuff allowed!";
  }
  ref_cnt++;

  //TWiFiStuff::dev_name = dev_name;
  time_cli.begin();

  pTgmBot = new TTgmBot(dev_name, p_conf, cb_change_formula);

  xDtorMutex = xSemaphoreCreateMutex();
  //xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 7500, this,
  xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 8000, this,
    (tskIDLE_PRIORITY + 2), &h_task, portNUM_PROCESSORS - 2);
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
  if(h_task)
  {
    vTaskDelete(h_task);
  }
  
  if(pTgmBot)
  {
    delete pTgmBot;
  }

  --ref_cnt;
}

void TWiFiStuff::tgb_send(const String& m, bool isMarkdownEnabled)
{
  if(pTgmBot)
  {
    pTgmBot->send(m, isMarkdownEnabled);
  }
}
}
