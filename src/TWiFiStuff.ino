#include "TWiFiStuff.h"

namespace WiFiStuff
{
int TWiFiStuff::ref_cnt = 0;

void TWiFiStuff::task(void *p)
{
  TWiFiStuff *pthis = static_cast<TWiFiStuff *>(p);
  TTgmBot *pTgmBot = pthis->pTgmBot;
  TWebSrv *pWebSrv = pthis->pWebSrv;

  for(;;)
  {
    if (pTgmBot)
      pTgmBot->run();

    if(pWebSrv)
      pWebSrv->run();

    vTaskDelay(10);
    //vTaskDelay(300);
    //yield();
  }
}

TWiFiStuff::TWiFiStuff(string dev_name, TPrefs *p_prefs):
  dev_name(dev_name),
  h_task(NULL),
  pTgmBot(NULL),
  pWebSrv(NULL)
{
  if(ref_cnt)
  {
    throw "Only one instance of TWiFiStuff allowed!";
  }
  ref_cnt++;

  pTgmBot = new TTgmBot(dev_name, p_prefs);
  pWebSrv = new TWebSrv();

  xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 3000, this,
    (tskIDLE_PRIORITY + 2), &h_task, portNUM_PROCESSORS - 2);
}

TWiFiStuff::~TWiFiStuff()
{
  if(h_task)
    vTaskDelete(h_task);
  --ref_cnt;
  if(pWebSrv)
    delete pWebSrv;
  if(pTgmBot)
    delete pTgmBot;
}
}
