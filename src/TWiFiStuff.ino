#include "TWiFiStuff.h"
#include "TMyApplication.h"

namespace WiFiStuff
{
using namespace MyApplication;

int TWiFiStuff::ref_cnt = 0;
String TWiFiStuff::dev_name;
TaskHandle_t TWiFiStuff::h_task = NULL;
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

  TWiFiStuff::dev_name = dev_name;
  time_cli.begin();

  pTgmBot = new TTgmBot(dev_name, p_conf, cb_change_formula);
  
  //xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 7500, this,
  xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 8000, this,
    (tskIDLE_PRIORITY + 2), &h_task, portNUM_PROCESSORS - 2);
}

TWiFiStuff::~TWiFiStuff()
{
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
