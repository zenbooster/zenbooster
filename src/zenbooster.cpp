#include <esp_coexist.h>
#include "TMyApplication.h"

/*#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif*/

using namespace MyApplication;

TMyApplication *p_app = NULL;
#if !CONFIG_AUTOSTART_ARDUINO
extern "C" void app_main()
{
  // initialize arduino library before we start the tasks
  initArduino();

  esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
  Serial.begin(115200);
  p_app = new TMyApplication();
}
#else
void setup()
{
  esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
  Serial.begin(115200);
  p_app = new TMyApplication();
}

void loop()
{
  p_app->run();
}
#endif