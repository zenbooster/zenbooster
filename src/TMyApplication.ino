#include "Version.h"
#include <Arduino.h>
#include "TCalcFormula.h"
#include "TBluetoothStuff.h"
#include "TWiFiStuff.h"
#include "TPrefs.h"
#include "TFormulaDB.h"
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
using namespace CalcFormula;
#ifdef SOUND
using namespace Noise;
#endif
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace Prefs;
using namespace FormulaDB;
using namespace common;
using namespace Util;

//
// SerialPrintf
// Реализует функциональность printf в Serial.print
// Применяется для отладочной печати
// Параметры как у printf
// Возвращает 
//    0 - ошибка формата
//    отрицательное чило - нехватка памяти, модуль числа равен запрашиваемой памяти
//    положительное число - количество символов, выведенное в Serial
//
/*const size_t SerialPrintf (const char *szFormat, ...)
{
  va_list argptr;
  va_start(argptr, szFormat);
  char *szBuffer = 0;
  const size_t nBufferLength = vsnprintf(szBuffer, 0, szFormat, argptr) + 1;
  if (nBufferLength == 1) return 0;
  szBuffer = (char *) malloc(nBufferLength);
  if (! szBuffer) return - nBufferLength;
  vsnprintf(szBuffer, nBufferLength, szFormat, argptr);
  Serial.print(szBuffer);
  free(szBuffer);
  return nBufferLength - 1;
} // const size_t SerialPrintf (const char *szFormat, ...)
*/
const char *TMyApplication::DEVICE_NAME_FULL = "zenbooster device";
const char *TMyApplication::DEVICE_NAME = "zenbooster-dev";
const char *TMyApplication::WIFI_SSID = DEVICE_NAME;
const char *TMyApplication::WIFI_PASS = "zbdzbdzbd";
int TMyApplication::MED_THRESHOLD;
int TMyApplication::MED_PRE_TRESHOLD_DELTA;
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

void TMyApplication::callback(const TRingBufferInItem rbi, void *arg)
{
  TMyApplication *p_this = (TMyApplication *)arg;

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
  
  p_this->ring_buffer_in[p_this->ring_buffer_in_index] = rbi;

  if(p_this->ring_buffer_in_size < 4)
    p_this->ring_buffer_in_size++;

  int med = p_this->calc_formula_meditation();
  p_this->ring_buffer_in_index = (p_this->ring_buffer_in_index + 1) & 3;
  
  Serial.printf("poor=%d, d=%d, t=%d, al=%d, ah=%d, bl=%d, bh=%d, gl=%d, gm=%d; em=%d; ea=%d; --> f_med=%d\n",
    rbi.poor_signal, rbi.delta, rbi.theta, rbi.alpha_lo, rbi.alpha_hi, rbi.beta_lo, rbi.beta_hi, rbi.gamma_lo, rbi.gamma_md, rbi.esense_med, rbi.esense_att, med);

  if(p_this->is_log_data_to_bot)
  {
    p_this->p_wifi_stuff->tgb_send(
      "`poor_signal="+String(rbi.poor_signal)+
      ", d="+String(rbi.delta)+", t="+String(rbi.theta)+
      ", al="+String(rbi.alpha_lo)+", ah="+String(rbi.alpha_hi)+
      ", bl="+String(rbi.beta_lo)+", bh="+String(rbi.beta_hi)+
      ", gl="+String(rbi.gamma_lo)+", gm="+String(rbi.gamma_md)+
      ", em="+String(rbi.esense_med)+", ea="+String(rbi.esense_att)+
      ", f="+String(med)+"`"
    );
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
} // void TMyApplication::callback(uint8_t code, uint8_t *data, void *arg)

void TMyApplication::update_calc_formula(TCalcFormula *pcf)
{
  xSemaphoreTake(xCFSemaphore, portMAX_DELAY);
  if (p_calc_formula)
  {
    delete p_calc_formula;
  }
  p_calc_formula = pcf;
  xSemaphoreGive(xCFSemaphore);
}

TMyApplication::TMyApplication():
  p_prefs(new TPrefs(DEVICE_NAME)),
  p_fdb(NULL),
  ring_buffer_in({}),
  ring_buffer_in_index(0),
  ring_buffer_in_size(0)
  /*, ring_buffer_out({})
  , ring_buffer_out_index(0)*/
#ifdef SOUND
  , p_noise(NULL)
#endif
{
  Serial.println(get_version_string());
  p_prefs->init_key("wop", "wake on power \\- проснуться при возобновлении питания \\(bool\\)",
  #ifdef LILYGO_WATCH_2020_V2
    "true",
  #else
    "false",
  #endif
    [](const String& value) -> void
  {
    TUtil::chk_value_is_bool(value);

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.printf("wakeup_reason=%d\n", wakeup_reason);
    esp_reset_reason_t reset_reason = esp_reset_reason();
    Serial.printf("reset_reason=%d\n", reset_reason);

    if(reset_reason != ESP_RST_POWERON && reset_reason != ESP_RST_DEEPSLEEP)
    {
      if(reset_reason == ESP_RST_SW)
      {
        Serial.println("Перезагрузились по требованию.");
      }
    }
    else
    {
      if(value == "false" && wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED)
      {
        Serial.println("Согласно настройкам, засыпаем по возобновлении питания...");
        pinMode(14, OUTPUT); // после загрузки, и после перехода в сон, именно на этом пине может остаться маленькое напряжение,
        digitalWrite(14, LOW); // и оно там останется даже во сне! Поэтому сбрасываем.
        esp_deep_sleep_start();
      }
      Serial.println("Проснулись по нажатию кнопки.");
    }
  });

  p_prefs->init_key("tr", "установка порога", "100", [](const String& value) -> void
  {
    TUtil::chk_value_is_number(value);

    MED_THRESHOLD = atoi(value.c_str());
  });

  p_prefs->init_key("trdt", "определяет, за сколько пунктов до порога уменьшать громкость шума", "60", [](const String& value) -> void
  {
    TUtil::chk_value_is_number(value);

    MED_PRE_TRESHOLD_DELTA = atoi(value.c_str());
  });

#ifdef SOUND
  p_prefs->init_key("mnl", "максимальная громкость шума \\(numeric\\)", "0.6", [](const String& value) -> void
  {
    TUtil::chk_value_is_numeric(value);

    float old_lvl = TNoise::get_level();
    float old_mnl = TNoise::MAX_NOISE_LEVEL;
    TNoise::MAX_NOISE_LEVEL = atof(value.c_str());
    TNoise::set_level(old_mnl ? (TNoise::MAX_NOISE_LEVEL * old_lvl) / old_mnl : TNoise::MAX_NOISE_LEVEL);
  });
#endif
  p_prefs->init_key("bod", "blink on data \\- мигнуть при поступлении нового пакета от гарнитуры \\(bool\\)", "false", [](const String& value) -> void
  {
    TUtil::chk_value_is_bool(value);

    is_blink_on_packets = (value == "true");
  });

  xCFSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xCFSemaphore);

  p_fdb = new TFormulaDB();

  if(p_fdb->is_empty())
  {
    Serial.println("Выполняем первичную инициализацию базы формул.");
    p_fdb->assign("gamma", "75 * 3 * gl / (gm + bh + bl)");
  }

  p_prefs->init_key("f", "формула", "diss", [this](const String& value) -> void
  {
    String val = this->p_fdb->get_value(value);
    
    TCalcFormula *pcf = TCalcFormula::compile(val);
    update_calc_formula(pcf);
  });

  p_prefs->init_key("ld", "log data \\- отправлять уровни ритмов, приходящих от гарнитуры \\(bool\\)", "false", [](const String& value) -> void
  {
    TUtil::chk_value_is_bool(value);

    is_log_data_to_bot = (value == "true");
  });

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

  p_wifi_stuff = new TWiFiStuff(DEVICE_NAME, p_prefs, p_fdb, [this](TCalcFormula *pcf) -> void
  {
    update_calc_formula(pcf);
  });
  p_bluetooth_stuff = new TBluetoothStuff(DEVICE_NAME, this, callback);
#ifdef SOUND
  p_noise = new TNoise();
#endif
#ifdef PIN_BTN
  ledcWrite(0, 0x40);
#endif
}

TMyApplication::~TMyApplication()
{
  if(p_fdb)
    delete p_fdb;
  if(p_prefs)
    delete p_prefs;
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