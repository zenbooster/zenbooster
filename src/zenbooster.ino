#include <esp_coexist.h>
#include "TMyApplication.h"

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

/*#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // DevKit v1
#endif*/

using namespace MyApplication;

TMyApplication *p_app = NULL;
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
