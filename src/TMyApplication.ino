#include "Version.h"
#include <Arduino.h>
#include "TWiFiStuff.h"
#include "TConf.h"
#include "TCalcFormula.h"
#include <limits>
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
bool TMyApplication::is_hard_shutdown = false;
TSleepMode *TMyApplication::p_sleep_mode = NULL;
WiFiManager TMyApplication::wifiManager;
int TMyApplication::threshold;
int TMyApplication::pre_threshold;
TRingBufferInItem TMyApplication::ring_buffer_in[4] = {};
int TMyApplication::ring_buffer_in_index = 0;
int TMyApplication::ring_buffer_in_size = 0;
#ifdef SOUND
TNoise *TMyApplication::p_noise = NULL;
#endif
TBluetoothStuff *TMyApplication::p_bluetooth_stuff = NULL;
TWiFiStuff *TMyApplication::p_wifi_stuff = NULL;
TCalcFormula *TMyApplication::p_calc_formula = NULL;
SemaphoreHandle_t TMyApplication::xOptRcMutex;
bool TMyApplication::is_log_data_to_bot = false;
bool TMyApplication::is_use_poor_signal = false;
#ifdef PIN_BTN
TButtonIllumination *TMyApplication::p_btn_il = NULL;
#endif
TMedSession *TMyApplication::p_med_session = NULL;

String TMyApplication::get_version_string(void)
{
  return String(DEVICE_NAME_FULL) + ", версия прошивки " VERSION ", дата сборки " BUILD_TIMESTAMP;
}

int TMyApplication::calc_formula_meditation()
{
  double res = 0;

  xSemaphoreTakeRecursive(xOptRcMutex, portMAX_DELAY);
  for(int i = 0; i < ring_buffer_in_size; i++)
  {
    TRingBufferInItem *p_item = ring_buffer_in + ((ring_buffer_in_index - i) & 3);
    if (!p_calc_formula)
    {
      throw String("Объект формулы (TCalcFormula) равен NULL!");
    }
    TRingBufferInItem *pcf = p_calc_formula;
    *pcf = *p_item; // копируем уровни ритмов
    float m = p_calc_formula->run();
    res += (m == numeric_limits<float>::infinity()) ? 0 : m;
  }
  xSemaphoreGiveRecursive(xOptRcMutex);
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
    {
      Serial.println("TMyApplication::callback: TCallbackEvent::eConnect");
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      bool is_use = is_use_poor_signal;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
      if(!is_use)
      {
        p_med_session = new TMedSession();
      #ifdef PIN_BTN
        TButtonIllumination::on_msession_connect();
      #endif
      }
      break;
    }

    case TCallbackEvent::eDisconnect:
      Serial.println("TMyApplication::callback: TCallbackEvent::eDisconnect");
      if(p_med_session)
      {
        delete p_med_session;
        p_med_session = NULL;
      }
    #ifdef SOUND
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TNoise::set_level(TNoise::MAX_NOISE_LEVEL);
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    #endif
    #ifdef PIN_BTN
      TButtonIllumination::on_msession_disconnect();
    #endif
      break;

    case TCallbackEvent::eData:
    {
      ring_buffer_in[ring_buffer_in_index] = *p_tpv;

      if(ring_buffer_in_size < 4)
        ring_buffer_in_size++;

      int med = calc_formula_meditation();
      ring_buffer_in_index = (ring_buffer_in_index + 1) & 3;
      
      String s = p_tpv->serialize() + "; --> f=" + med;
      Serial.println(s);

      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      bool is_log = is_log_data_to_bot;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
      if(is_log)
      {
        TWiFiStuff::tgb_send("`" + s + "`");
      }

      // подсветка:
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      bool is_use = is_use_poor_signal;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
      if(is_use) // если отслеживаем POOR_SIGNAL
      {
        if(p_tpv->is_has_poor && p_tpv->poor) // если сигнал плохой
        {
          if(p_med_session)
          {
            callback(p_tpv, TCallbackEvent::eDisconnect);
          }
        #ifdef PIN_BTN
          TButtonIllumination::on_msession_poor_signal();
        #endif
        }
        else
        {
          if(!p_med_session)
          {
          #ifdef PIN_BTN
            TButtonIllumination::on_msession_connect();
          #endif
            p_med_session = new TMedSession();
          }
        }
      }

    #ifdef PIN_BTN
      TButtonIllumination::on_msession_data();
    #endif
      if(TButtonIllumination::is_poor_signal_indicated)
      {
        return;
      }

      p_med_session->calc_next(med);

      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      int tr = TMyApplication::threshold;
      int pretr = TMyApplication::pre_threshold;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);

      if(med >= tr)
      {
      #ifdef SOUND
        TNoise::set_level(0);
      #endif
      #ifdef PIN_BTN
        TButtonIllumination::on_threshold_reached();
      #endif
      }
      else
      {
        int d = tr - med;
        if(d < pretr)
        {
        #ifdef SOUND
          xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
          TNoise::set_level(((float)d * TNoise::MAX_NOISE_LEVEL) / (float)pretr);
          xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
        #endif
        #ifdef PIN_BTN
          TButtonIllumination::on_pre_threshold_reached(d, pretr);
        #endif
        }
        else
        {
        #ifdef SOUND
          xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
          TNoise::set_level(TNoise::MAX_NOISE_LEVEL);
          xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
        #endif
        #ifdef PIN_BTN
          TButtonIllumination::on_pre_threshold_not_reached();
        #endif
        }
      }
      break;
    } // case TCallbackEvent::eData:
  } // switch(evt)
} // void TMyApplication::callback(uint8_t code, uint8_t *data, void *arg)

void TMyApplication::update_calc_formula(TCalcFormula *pcf)
{
  if(p_calc_formula)
  {
    if(p_med_session)
    {
      TMyApplication::callback(NULL, TCallbackEvent::eDisconnect);
      delete p_calc_formula;
      TMyApplication::callback(NULL, TCallbackEvent::eConnect);
    }
    else
    {
      delete p_calc_formula;
    }
  }
  p_calc_formula = pcf;
}

TMyApplication::TMyApplication()
  /*, ring_buffer_out({})
  , ring_buffer_out_index(0)*/
{
  Serial.println(get_version_string());

  TCbSleepFunction cb = [this]() -> void
  {
    Serial.println("TCbSleepFunction: begin");
    delete this;
    Serial.println("TCbSleepFunction: end");
  };

  p_sleep_mode = new TSleepMode(is_hard_shutdown ? NULL : cb);

  p_conf = new TConf();
#ifdef PIN_BTN
  p_btn_il = new TButtonIllumination();
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
  TButtonIllumination::on_wait_for_connect();
#endif
}

TMyApplication::~TMyApplication()
{
#ifdef PIN_BTN
  if(p_btn_il)
    // это запустит индикацию завершения работы:
    delete p_btn_il;
#endif

#ifdef SOUND
  if(p_noise)
    delete p_noise;
#endif

  if(p_bluetooth_stuff)
    delete p_bluetooth_stuff;

  if(p_wifi_stuff)
    delete p_wifi_stuff;

  if(p_conf)
    delete p_conf;
}

void TMyApplication::run(void)
{
  //vTaskDelay(portMAX_DELAY);
  vTaskDelete(NULL);
}
}