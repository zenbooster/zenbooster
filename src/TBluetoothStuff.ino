#include "TBluetoothStuff.h"
#include "TNoise.h"
#include "freertos\\task.h"

namespace BluetoothStuff
{
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
        if(!pthis->connected) // были отключены, а теперь подключились
        {
          ledcWrite(2, 255);
        }
        // Synchronize on [SYNC] bytes
        try
        {
        pthis->p_tpp->run();
        }
        catch(exception e)
        {
        // это, например, если во время чтения пропала связь с устройством, и мы бросили исключение...
        }
        catch(string e)
        {
        Serial.println(e.c_str());
        }
    }
    else
    if (!pthis->SerialBT.connected())
    {
        digitalWrite(LED_BUILTIN, LOW);
        ledcWrite(2, 0);
        TNoise::set_level(TNoise::MAX_NOISE_LEVEL);

        if(pthis->connected) // были подключены, а теперь отключились
        {
          //ESP.restart(); // Запарил...)
          ledcWrite(0, 0x40);
          ledcWrite(1, 0);
        }

        pthis->connected = pthis->SerialBT.connect(pthis->address);//, 1, ESP_SPP_SEC_AUTHENTICATE);
        //connected = SerialBT.connect(name);
        
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

TBluetoothStuff::TBluetoothStuff(string dev_name, TMyApplication *p_app, tpfn_callback pfn_callback):
  dev_name(dev_name),
  p_app(p_app),
  pfn_callback(pfn_callback),
  MACadd("20:21:04:08:39:93"),
  address({0x20, 0x21, 0x04, 0x08, 0x39, 0x93}),
  name("MindWave"),
  pin("0000"),
  connected(false),
  p_tpp(NULL)
{
  if(ref_cnt)
  {
    throw "Only one instance of TBluetoothStuff allowed!";
  }
  ref_cnt++;

  p_tpp = new TTgamPacketParser(&SerialBT, pfn_callback, p_app);

  SerialBT.setPin(pin.c_str());
  SerialBT.begin(dev_name.c_str(), true);
  //SerialBT.setPin(pin.c_str());
  Serial.println(F("The device started in master mode, make sure remote BT device is on!"));

  int res = xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 5000, this,
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
