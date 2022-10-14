#include "Version.h"
#include <Arduino.h>
#include "TWiFiStuff.h"
#include "TConf.h"
#include "TCalcFormula.h"
#include <sstream>
#include <exception>
#include "common.h"
#include "TUtil.h"
#ifdef SOUND
# include "TNoise.h"
#endif
#include "TMyApplication.h"

namespace MyApplication
{
using namespace std;
using namespace common;
using namespace Util;

const char *TMyApplication::DEVICE_NAME_FULL = "zenbooster device";
const char *TMyApplication::DEVICE_NAME = "zenbooster-dev";
const char *TMyApplication::WIFI_SSID = DEVICE_NAME;
const char *TMyApplication::WIFI_PASS = "zbdzbdzbd";
TConf *TMyApplication::p_conf = NULL;
int TMyApplication::MED_THRESHOLD;
int TMyApplication::MED_PRE_TRESHOLD_DELTA;
TRingBufferInItem TMyApplication::ring_buffer_in[4] = {};
int TMyApplication::ring_buffer_in_index = 0;
int TMyApplication::ring_buffer_in_size = 0;
#ifdef SOUND
TNoise *TMyApplication::p_noise = NULL;
#endif
TBluetoothStuff *TMyApplication::p_bluetooth_stuff = NULL;
TWiFiStuff *TMyApplication::p_wifi_stuff = NULL;
TCalcFormula *TMyApplication::p_calc_formula = NULL;
SemaphoreHandle_t TMyApplication::xCFSemaphore;
bool TMyApplication::is_blink_on_packets = false;
bool TMyApplication::is_blue_pulse = true;
bool TMyApplication::is_log_data_to_bot = false;

String TMyApplication::get_version_string(void)
{
  return String(DEVICE_NAME_FULL) + ", версия прошивки " VERSION ", дата сборки " BUILD_TIMESTAMP;
}

int TMyApplication::calc_formula_meditation()
{
  double res = 0;

  xSemaphoreTake(xCFSemaphore, portMAX_DELAY);
  for(int i = 0; i < ring_buffer_in_size; i++)
  {
    TRingBufferInItem *p_item = ring_buffer_in + ((ring_buffer_in_index - i) & 3);
    if (!p_calc_formula)
    {
      throw String("Объект формулы (TCalcFormula) равен NULL!");
    }
    TRingBufferInItem *pcf = p_calc_formula;
    *pcf = *p_item; // копируем уровни ритмов
    res += p_calc_formula->run();
  }
  xSemaphoreGive(xCFSemaphore);
  return res / ring_buffer_in_size;
}

int TMyApplication::int_from_12bit(const uint8_t *buf)
{
  return (*buf << 16) + (buf[1] << 8) + buf[2];
}

void TMyApplication::callback(const TTgamParsedValues *p_tpv, TCallbackEvent evt)
{
  switch(evt)
  {
    case TCallbackEvent::eConnect:
      Serial.println("TMyApplication::callback: TCallbackEvent::eConnect");
      break;

    case TCallbackEvent::eDisconnect:
      Serial.println("TMyApplication::callback: TCallbackEvent::eDisconnect");
      break;

    case TCallbackEvent::eData:
    {
    #ifdef PIN_BTN
      if(is_blink_on_packets)
      {
        ledcWrite(2, is_blue_pulse ? 255: 128);
        is_blue_pulse = !is_blue_pulse;
      }
      else
      {
        ledcWrite(2, 255);
      }
    #endif
      
      ring_buffer_in[ring_buffer_in_index] = *p_tpv;

      if(ring_buffer_in_size < 4)
        ring_buffer_in_size++;

      int med = calc_formula_meditation();
      ring_buffer_in_index = (ring_buffer_in_index + 1) & 3;
      
      String s = p_tpv->serialize() + "; --> f=" + med;
      Serial.println(s);

      if(is_log_data_to_bot)
      {
        p_wifi_stuff->tgb_send("`" + s + "`");
      }

      if(med > TMyApplication::MED_THRESHOLD)
      {
      #ifdef SOUND
        TNoise::set_level(0);
      #endif
      #ifdef PIN_BTN
        ledcWrite(0, 255);
        ledcWrite(1, 255);
      #endif
      }
      else
      {
        int d = TMyApplication::MED_THRESHOLD - med;

        if(d < TMyApplication::MED_PRE_TRESHOLD_DELTA)
        {
        #ifdef SOUND
          TNoise::set_level(((float)d * TNoise::MAX_NOISE_LEVEL) / (float)TMyApplication::MED_PRE_TRESHOLD_DELTA);
        #endif
        #ifdef PIN_BTN
          int led_lvl = 255 - (d * 255) / TMyApplication::MED_PRE_TRESHOLD_DELTA;
          ledcWrite(0, led_lvl);
          ledcWrite(1, led_lvl);
        #endif
        }
        else
        {
        #ifdef SOUND
          TNoise::set_level(TNoise::MAX_NOISE_LEVEL);
        #endif
        #ifdef PIN_BTN
          ledcWrite(0, 0);
          ledcWrite(1, 0);
        #endif
        }
      }
      break;
    } // case TCallbackEvent::eData:
  } // switch(evt)
} // void TMyApplication::callback(uint8_t code, uint8_t *data, void *arg)

void TMyApplication::update_calc_formula(TCalcFormula *pcf)
{
  xSemaphoreTake(xCFSemaphore, portMAX_DELAY);
  if (p_calc_formula)
  {
    TMyApplication::callback(NULL, TCallbackEvent::eDisconnect);
    delete p_calc_formula;
    TMyApplication::callback(NULL, TCallbackEvent::eConnect);
  }
  p_calc_formula = pcf;
  xSemaphoreGive(xCFSemaphore);
}

TMyApplication::TMyApplication()
  /*, ring_buffer_out({})
  , ring_buffer_out_index(0)*/
{
  Serial.println(get_version_string());
  p_conf = new TConf(this);

#ifdef PIN_BTN
  ledcSetup(0, 40, 8);
  ledcSetup(1, 40, 8);
  ledcSetup(2, 40, 8);

  ledcAttachPin(PIN_LED_R, 0);
  ledcAttachPin(PIN_LED_G, 1);
  ledcAttachPin(PIN_LED_B, 2);

  ledcWrite(0, 0xff);
#endif
  pinMode(LED_BUILTIN, OUTPUT);

  // инициализация WiFi:
  // Временно запретим wifi, т.к. есть проблемы сосуществования wifi и bluetooth на одном радио.
  // Ждём модуль bluetooth HC-06 - он должен рещить все проблемы.
  wifiManager.setHostname(DEVICE_NAME);
  //wifiManager.startConfigPortal(WIFI_SSID, WIFI_PASS);
  wifiManager.autoConnect(WIFI_SSID, WIFI_PASS);

  p_wifi_stuff = new TWiFiStuff(DEVICE_NAME, p_conf, [](TCalcFormula *pcf) -> void
  {
    update_calc_formula(pcf);
  });
  p_bluetooth_stuff = new TBluetoothStuff(DEVICE_NAME, callback);
#ifdef SOUND
  p_noise = new TNoise();
#endif
#ifdef PIN_BTN
  ledcWrite(0, 0x40);
#endif
}

TMyApplication::~TMyApplication()
{
  if(p_conf)
    delete p_conf;
  if(p_bluetooth_stuff)
    delete p_bluetooth_stuff;
  if(p_wifi_stuff)
    delete p_wifi_stuff;
#ifdef SOUND
  if(p_noise)
    delete p_noise;
#endif
}

void TMyApplication::run(void)
{
  //vTaskDelay(portMAX_DELAY);
  vTaskDelete(NULL);
}
}