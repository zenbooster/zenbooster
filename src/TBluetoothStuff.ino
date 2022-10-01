#include "TBluetoothStuff.h"
#include "common.h"
#include "TUtil.h"
#include "TNoise.h"
#include "freertos\\task.h"

namespace BluetoothStuff
{
using namespace Util;
using namespace Noise;

int TBluetoothStuff::ref_cnt = 0;

void TBluetoothStuff::task(void *p)
{
  TBluetoothStuff *pthis = static_cast<TBluetoothStuff *>(p);

  for(;;)
  {
    if (pthis->SerialBT.available())
    {
      digitalWrite(LED_BUILTIN, HIGH);
      // Synchronize on [SYNC] bytes
      try
      {
      pthis->p_tpp->run();
      }
      catch(exception e)
      {
      // это, например, если во время чтения пропала связь с устройством, и мы бросили исключение...
      }
      catch(String e)
      {
      Serial.println(e.c_str());
      }
    }
    else
    if (!pthis->SerialBT.connected())
    {
      TNoise::set_level(TNoise::MAX_NOISE_LEVEL);

      if(pthis->connected) // были подключены, а теперь отключились
      {
      #ifdef PIN_BTN
        ledcWrite(0, 0x40);
        ledcWrite(1, 0);
        ledcWrite(2, 0);
      #endif
        digitalWrite(LED_BUILTIN, LOW);
      }

      pthis->connected = pthis->SerialBT.connect(pthis->address);//, 1, ESP_SPP_SEC_AUTHENTICATE);
      
      if(pthis->connected)
      {
        Serial.println("Connected Succesfully!");
      }
      else
      {
        Serial.println(F("Failed to connect. Make sure remote device is available and in range."));
      }
    }
    yield();
  }
}

TBluetoothStuff::TBluetoothStuff(String dev_name, TMyApplication *p_app, tpfn_callback pfn_callback):
  dev_name(dev_name),
  p_app(p_app),
  pfn_callback(pfn_callback)
{
  p_tpp = NULL;

  if(ref_cnt)
  {
    throw "Only one instance of TBluetoothStuff allowed!";
  }
  ref_cnt++;

  connected = false;
  pin = "0000";
  name = "MindWave";
  MACadd = "20:21:04:08:39:93";
  TUtil::mac_2_array(MACadd, address);

  p_tpp = new TTgamPacketParser(&SerialBT, pfn_callback, p_app);

  SerialBT.setPin(pin.c_str());
  SerialBT.begin(dev_name, true);
  Serial.println(F("The device started in master mode, make sure remote BT device is on!"));

  //xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 1900, this,
  xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 1800, this,
  //xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 2500, this,
      (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 1);
}

TBluetoothStuff::~TBluetoothStuff()
{
  // сделать удаление задачи.
  --ref_cnt;
    if(p_tpp)
        delete p_tpp;
}
}
