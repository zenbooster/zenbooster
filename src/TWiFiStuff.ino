#include "TWiFiStuff.h"

namespace WiFiStuff
{
int TWiFiStuff::ref_cnt = 0;

void TWiFiStuff::task(void *p)
{
  TWiFiStuff *pthis = static_cast<TWiFiStuff *>(p);
  TTgmBot *pTgmBot = pthis->pTgmBot;

  for(;;)
  {
    if (pTgmBot)
      pTgmBot->run();

    yield();
  }
}

TWiFiStuff::TWiFiStuff(String dev_name, TConf *p_conf, TgmBot::TCbChangeFunction cb_change_formula):
  dev_name(dev_name),
  h_task(NULL),
  pTgmBot(NULL)
{
  if(ref_cnt)
  {
    throw "Only one instance of TWiFiStuff allowed!";
  }
  ref_cnt++;

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
