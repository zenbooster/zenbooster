#include "TBluetoothStuff.h"
#include "TConf.h"
#include "TButtonIllumination.h"
#include "common.h"
#include "TUtil.h"
#include "TNoise.h"
#include "TWorker/TWorker.h"
#include "TWorker/TWorkerTaskProcessTgam.h"
#include "freertos/task.h"
#include <esp_spp_api.h>

namespace BluetoothStuff
{
using namespace MyApplication;
using namespace ButtonIllumination;
using namespace common;
using namespace Util;
using namespace Noise;
using namespace Worker;

TTask *TBluetoothStuff::p_task = NULL;
// используется в т.ч. для инициализации "mil":
SemaphoreHandle_t TBluetoothStuff::xConnSemaphore = NULL;
bool TBluetoothStuff::is_connected = false;
tpfn_callback TBluetoothStuff::pfn_callback = NULL;
uint8_t TBluetoothStuff::address[6] = {};
TTgamPacketParser *TBluetoothStuff::p_tpp = NULL;

const char *TBluetoothStuff::get_class_name()
{
  return "TBluetoothStuff";
}

void TBluetoothStuff::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_OPEN_EVT)
  {
    uint32_t handle = param->open.handle;
    TWorker::printf("callback: ESP_SPP_OPEN_EVT, handle=%u\n", handle);

    // Тут семафор не нужен, т.к. событие происходит до выхода из функции connect.
    TBluetoothStuff::is_connected = true;
    TBluetoothStuff::pfn_callback(NULL, TCallbackEvent::eConnect);
    TWorker::println("Успешно подключились!");
  }
  else
  if (event == ESP_SPP_CLOSE_EVT)
  {
    uint32_t handle = param->close.handle;
    // Обработчик закрытия может быть вызван в тот момент, когда будет выполняться
    // задача TWorkerTaskTerminate, по этому вызываем через Serial:
    Serial.printf("callback: ESP_SPP_CLOSE_EVT, handle=%u\n", handle);

    // В данный момент никто не может изменять is_connected, по этому и семафор не нужен.
    if(TBluetoothStuff::is_connected) // были подключены, а теперь отключились
    {
    #ifdef PIN_BTN
      TButtonIllumination::on_wait_for_connect();
    #endif
      digitalWrite(LED_BUILTIN, LOW);
    }

    // А вот тут кое-кто может проверять состояние xConnSemaphore, по этому используем семафор.
    TBluetoothStuff::pfn_callback(NULL, TCallbackEvent::eDisconnect);
    xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
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
    // В этот момент, обработчик ESP_SPP_CLOSE_EVT может изменять is_connected.
    xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
    is_conn = is_connected;
    xSemaphoreGive(xConnSemaphore);

    if(!is_conn)
    {
      uint8_t addr[6];
      xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
      memcpy(addr, address, 6); // делаем копию
      xSemaphoreGiveRecursive(TConf::xOptRcMutex);

      bool is_was_connected = pthis->SerialBT.connect(addr);
      if(!is_was_connected)
      {
        TWorker::println("Ошибка подключения. Убедитесь, что удалённое устройство включено и доступно.");
      }
    }
    vTaskDelay(100);
  }
}

TBluetoothStuff::TBluetoothStuff(String dev_name, tpfn_callback pfn_callback)
{
  TBluetoothStuff::pfn_callback = pfn_callback;
  p_tpp = NULL;

  pin = "0000";
  //name = "MindWave";
  xConnSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xConnSemaphore);

  p_tpp = new TTgamPacketParser(
    &SerialBT, 
    [](const TTgamParsedValues& tpv) -> void
    {
      TWorkerTaskProcessTgam *pwt = new TWorkerTaskProcessTgam(TBluetoothStuff::pfn_callback, tpv);
      TWorker::send(pwt);
    }
  );

  SerialBT.setPin(pin.c_str());
  SerialBT.register_callback(callback);
  SerialBT.onData(
    [](const uint8_t *buffer, size_t size) -> void
    {
      //if (dp)
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
  TWorker::println("Устройство запущено в режиме мастера, убедитесь, что удалённое устройство включено!");

  p_task = new TTask(task, "TBluetoothStuff", TBLUETOOTHSTUFF_TASK_STACK_SIZE, this, tskIDLE_PRIORITY + 2, portNUM_PROCESSORS - 1);
}

TBluetoothStuff::~TBluetoothStuff()
{
  bool is_conn;

  xSemaphoreTake(xConnSemaphore, portMAX_DELAY);
  is_conn = is_connected;
  //xSemaphoreGive(xConnSemaphore);

  if(p_task)
  {
    delete p_task;
  }
  xSemaphoreGive(xConnSemaphore);

  // Если соединение было установлено, разрываем
  // его, и ждём, когда отработают обработчики:
  if(is_conn)
  {
    // Все колбэки будут вызваны до выхода из метода disconnect:
    SerialBT.disconnect();
  }

  if(p_tpp)
      delete p_tpp;
  
  vSemaphoreDelete(xConnSemaphore);
}
}
