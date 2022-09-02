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
  }
}

TWiFiStuff::TWiFiStuff(string dev_name, TPrefs *p_prefs):
  dev_name(dev_name),
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

  xTaskCreatePinnedToCore(task, "TWiFiStuff::task", 15000, this,
    //(tskIDLE_PRIORITY + 3), NULL, portNUM_PROCESSORS - 1);
    (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 1);
}

TWiFiStuff::~TWiFiStuff()
{
  // сделать удаление задачи.
  --ref_cnt;
  if(pWebSrv)
    delete pWebSrv;
  if(pTgmBot)
    delete pTgmBot;
}
}
