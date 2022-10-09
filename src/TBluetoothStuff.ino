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

class TBluetoothDataProcessor
{
  private:
    QueueHandle_t queue;

    static void task(void *p);

  public:
    TBluetoothDataProcessor();

    void send(const TTgamParsedValues& tpv);
};

void TBluetoothDataProcessor::task(void *p)
{
  TBluetoothDataProcessor *p_this = static_cast<TBluetoothDataProcessor *>(p);

  for(;;)
  {
    TTgamParsedValues *p;
    if(pdTRUE == xQueueReceive(p_this->queue, &p, portMAX_DELAY))
    {
      TBluetoothStuff::pfn_callback(*p, TBluetoothStuff::p_app);
      delete p;
    }
  }
}

TBluetoothDataProcessor::TBluetoothDataProcessor()
{
  queue = xQueueCreate(64, sizeof(TTgamParsedValues*));

  if (queue == NULL) {
    throw String("Error creating the queue");
  }

  xTaskCreatePinnedToCore(task, "TBluetoothDataProcessor::task", 2000, this,
      (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 1);
}

void TBluetoothDataProcessor::send(const TTgamParsedValues& tpv)
{
    TTgamParsedValues *p = new TTgamParsedValues(tpv);
    xQueueSend(queue, &p, 0);
}

////////
int TBluetoothStuff::ref_cnt = 0;
SemaphoreHandle_t TBluetoothStuff::xConnSemaphore;
bool TBluetoothStuff::is_connected = false;
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
    xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
    TBluetoothStuff::is_connected = true;
    xSemaphoreGive(xConnSemaphore);
    Serial.println("Connected Succesfully!");
  }
  else
  if (event == ESP_SPP_CLOSE_EVT)
  {
    uint32_t handle = param->close.handle;
    Serial.printf("callback: ESP_SPP_CLOSE_EVT, handle=%u\n", handle);

    TNoise::set_level(TNoise::MAX_NOISE_LEVEL);

    xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
    if(TBluetoothStuff::is_connected) // были подключены, а теперь отключились
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
    TBluetoothStuff::is_connected = false;
    xSemaphoreGive(xConnSemaphore);
  }
}

void TBluetoothStuff::task(void *p)
{
  TBluetoothStuff *pthis = static_cast<TBluetoothStuff *>(p);
  bool is_conn;

  for(;;)
  {
    xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
    is_conn = is_connected;
    xSemaphoreGive(xConnSemaphore);

    if(!is_conn)
    {
      bool is_was_connected = pthis->SerialBT.connect(pthis->address);
      
      if(!is_was_connected)
      {
        Serial.println(F("Failed to connect. Make sure remote device is available and in range."));
      }
    }
    vTaskDelay(100);
  }
}

TBluetoothStuff::TBluetoothStuff(String dev_name, TMyApplication *p_app, tpfn_callback pfn_callback):
  dev_name(dev_name)
{
  if(ref_cnt)
  {
    throw "Only one instance of TBluetoothStuff allowed!";
  }
  ref_cnt++;

  TBluetoothStuff::pfn_callback = pfn_callback;
  TBluetoothStuff::p_app = p_app;
  p_tpp = NULL;

  pin = "0000";
  name = "MindWave";
  MACadd = "20:21:04:08:39:93";
  TUtil::mac_2_array(MACadd, address);

  xConnSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xConnSemaphore);

  p_tpp = new TTgamPacketParser(
    &SerialBT, 
    [](const TTgamParsedValues& tpv) -> void
    {
      dp->send(tpv);
    }
  );

  SerialBT.setPin(pin.c_str());
  SerialBT.register_callback(callback);
  SerialBT.onData(
    [](const uint8_t *buffer, size_t size) -> void
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
  );
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
