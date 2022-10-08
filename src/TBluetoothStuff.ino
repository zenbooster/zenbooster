#include "TBluetoothStuff.h"
#include "common.h"
#include "TUtil.h"
#include "TNoise.h"
#include "freertos\\task.h"
#include <esp_spp_api.h>

namespace BluetoothStuff
{
using namespace common;
using namespace Util;
using namespace Noise;

struct TBluetoothDataMessage
{
  const uint8_t *data;
  size_t size;

  TBluetoothDataMessage(const uint8_t *data, size_t size);
  ~TBluetoothDataMessage();
};

TBluetoothDataMessage::TBluetoothDataMessage(const uint8_t *data, size_t size):
  data(new uint8_t[size]),
  size(size)
{
  if(data)
  {
    memcpy((void *)this->data, data, size);
  }
}

TBluetoothDataMessage::~TBluetoothDataMessage()
{
  if(data)
  {
    delete [] data;
  }
}

class TBluetoothDataProcessor
{
  private:
    QueueHandle_t queue;

    static void task(void *p);

  public:
    TBluetoothDataProcessor();

    void send(const uint8_t *data, size_t size);
};

void TBluetoothDataProcessor::task(void *p)
{
  TBluetoothDataProcessor *p_this = static_cast<TBluetoothDataProcessor *>(p);

  for(;;)
  {
    TBluetoothDataMessage *p;
    if(pdTRUE == xQueueReceive(p_this->queue, &p, portMAX_DELAY))
    {
      TBluetoothStuff::pfn_callback(p->data, TBluetoothStuff::p_app);
      delete p;
    }
  }
}

TBluetoothDataProcessor::TBluetoothDataProcessor()
{
  queue = xQueueCreate(64, sizeof(TBluetoothDataMessage*));

  if (queue == NULL) {
    throw String("Error creating the queue");
  }

  xTaskCreatePinnedToCore(task, "TBluetoothDataProcessor::task", 2000, this,
      (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 1);
}

void TBluetoothDataProcessor::send(const uint8_t *data, size_t size)
{
    TBluetoothDataMessage *p = new TBluetoothDataMessage(data, size);
    xQueueSend(queue, &p, 0);
}

////////
int TBluetoothStuff::ref_cnt = 0;
bool TBluetoothStuff::connected = false;
TMyApplication *TBluetoothStuff::p_app;
tpfn_callback TBluetoothStuff::pfn_callback = NULL;
TTgamPacketParser *TBluetoothStuff::p_tpp = NULL;
TBluetoothDataProcessor *TBluetoothStuff::dp = NULL;

void TBluetoothStuff::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_OPEN_EVT)
  {
    uint32_t handle = param->open.handle;
    Serial.printf("callback: ESP_SPP_OPEN_EVT, handle=%u\n", handle);

    dp = new TBluetoothDataProcessor();

  }
  else
  if (event == ESP_SPP_CLOSE_EVT)
  {
    uint32_t handle = param->close.handle;
    Serial.printf("callback: ESP_SPP_CLOSE_EVT, handle=%u\n", handle);

    TNoise::set_level(TNoise::MAX_NOISE_LEVEL);

    if(TBluetoothStuff::connected) // были подключены, а теперь отключились
    {
    #ifdef PIN_BTN
      ledcWrite(0, 0x40);
      ledcWrite(1, 0);
      ledcWrite(2, 0);
    #endif
      digitalWrite(LED_BUILTIN, LOW);
    }

    delete dp;
    dp = NULL;
  }
}

void TBluetoothStuff::on_data(const uint8_t *buffer, size_t size)
{
  if (dp)
  {
    const uint8_t *data = buffer;
    const uint8_t *p_end = data + size;
    for(const uint8_t *p = data; p < p_end; p++)
    {
      uint8_t b = *p;
      if(p_tpp)
      {
        p_tpp->run(b);
      }
    }
  }
}

void TBluetoothStuff::task(void *p)
{
  TBluetoothStuff *pthis = static_cast<TBluetoothStuff *>(p);

  for(;;)
  {
    if(!pthis->SerialBT.connected())
    {
      connected = pthis->SerialBT.connect(pthis->address);
      
      if(connected)
      {
        Serial.println("Connected Succesfully!");
      }
      else
      {
        Serial.println(F("Failed to connect. Make sure remote device is available and in range."));
      }
    }
    //yield();
  }
}

TBluetoothStuff::TBluetoothStuff(String dev_name, TMyApplication *p_app, tpfn_callback pfn_callback):
  dev_name(dev_name)
{
  TBluetoothStuff::pfn_callback = pfn_callback;
  TBluetoothStuff::p_app = p_app;
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

  p_tpp = new TTgamPacketParser(
    &SerialBT, 
    [](const uint8_t *data, size_t size) -> void
    {
      dp->send(data, size);
    },
    pfn_callback,
    p_app
  );

  SerialBT.setPin(pin.c_str());
  SerialBT.register_callback(callback);
  SerialBT.onData(on_data);
  SerialBT.begin(dev_name, true);
  Serial.println(F("The device started in master mode, make sure remote BT device is on!"));

  //xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 1900, this,
  xTaskCreatePinnedToCore(task, "TBluetoothStuff::task", 2200, this,
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
