#include <Arduino.h>
#include <esp_coexist.h>
#include "TCalcFormula.h"
#include "WiFiManager.h"
#include "TBluetoothStuff.h"
#include "TWiFiStuff.h"
#include "TPrefs.h"
#include "TFormulaDB.h"
#include <sstream>
#include <exception>
#include "common.h"
#ifdef SOUND
# include "TNoise.h"
#endif
#ifdef PIN_BTN
# include "TSleepMode.h"
#endif

using namespace std;
using namespace CalcFormula;
#ifdef SOUND
using namespace Noise;
#endif
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace Prefs;
using namespace FormulaDB;
#ifdef PIN_BTN
using namespace SleepMode;
#endif
using namespace common;

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // DevKit v1
#endif

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
const size_t SerialPrintf (const char *szFormat, ...)
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

/*struct TRingBufferOutItem
{
  time_t time;
  int meditation;
};*/

class TMyApplication
{
  private:
    const char *DEVICE_NAME = "zenbooster-dev";
    const char *WIFI_SSID = DEVICE_NAME;
    const char *WIFI_PASS = "zbdzbdzbd";
    
    TPrefs *p_prefs;
    TFormulaDB *p_fdb;
#ifdef PIN_BTN
    TSleepMode SleepMode;
#endif
    WiFiManager wifiManager;

    static int MED_THRESHOLD;// = 95;
    static int MED_PRE_TRESHOLD_DELTA;// = 10;

    TRingBufferInItem ring_buffer_in[4];
    int ring_buffer_in_index;
    int ring_buffer_in_size;

    // размер подобрать под ширину отображаемого графика:
    //TRingBufferOutItem ring_buffer_out[1024];
    //TRingBufferOutItem ring_buffer_out[512];
    //int ring_buffer_out_index;
    
  #ifdef SOUND
    TNoise *p_noise;
  #endif
    TBluetoothStuff *p_bluetooth_stuff;
    TWiFiStuff *p_wifi_stuff;
    static TCalcFormula *p_calc_formula;
    static SemaphoreHandle_t xCFSemaphore;
    
    static bool is_blink_on_packets; // мигнуть при поступлении нового пакета от гарнитуры?
    static bool is_blue_pulse;

    int calc_formula_meditation();
    static int int_from_12bit(unsigned char *buf);
    static void callback(unsigned char code, unsigned char *data, void *arg);
    void update_calc_formula(TCalcFormula *pcf);

  public:
    TMyApplication();
    ~TMyApplication();

    void run(void);
};

int TMyApplication::MED_THRESHOLD;
int TMyApplication::MED_PRE_TRESHOLD_DELTA;
TCalcFormula *TMyApplication::p_calc_formula = NULL;
SemaphoreHandle_t TMyApplication::xCFSemaphore;
bool TMyApplication::is_blink_on_packets = false;
bool TMyApplication::is_blue_pulse = true;

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

int TMyApplication::int_from_12bit(unsigned char *buf)
{
  return (*buf << 16) + (buf[1] << 8) + buf[2];
}

void TMyApplication::callback(unsigned char code, unsigned char *data, void *arg)
{
  TMyApplication *p_this = (TMyApplication *)arg;

  if (code == 0x83)
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

    int delta = int_from_12bit(data);
    int theta = int_from_12bit(data + 3);
    int alpha_lo = int_from_12bit(data + 6);
    int alpha_hi = int_from_12bit(data + 9);
    int beta_lo = int_from_12bit(data + 12);
    int beta_hi = int_from_12bit(data + 15);
    //
    int gamma_lo = int_from_12bit(data + 18);
    int gamma_md = int_from_12bit(data + 21);

    time_t now;
    time(&now);
    {
      TRingBufferInItem item = {now, delta, theta, alpha_lo, alpha_hi, beta_lo, beta_hi, gamma_lo, gamma_md};
      p_this->ring_buffer_in[p_this->ring_buffer_in_index] = item;

      if(p_this->ring_buffer_in_size < 4)
        p_this->ring_buffer_in_size++;
    }

    int med = p_this->calc_formula_meditation();
    p_this->ring_buffer_in_index = (p_this->ring_buffer_in_index + 1) & 3;
    
    /*TRingBufferOutItem item = {now, med};
    p_this->ring_buffer_out[p_this->ring_buffer_out_index] = item;
    p_this->ring_buffer_out_index = (p_this->ring_buffer_out_index + 1) & 3;
    */

    SerialPrintf("delta=%d, theta=%d, alpha_lo=%d, alpha_hi=%d, beta_lo=%d, beta_hi=%d, gamma_lo=%d, gamma_md=%d; --> f_med=%d\n", delta, theta, alpha_lo, alpha_hi, beta_lo, beta_hi, gamma_lo, gamma_md, med);

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
  } // if (code == 0x83)
} // void TMyApplication::callback(unsigned char code, unsigned char *data, void *arg)

bool is_number(const String &s)
{
  return !s.isEmpty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void chk_value_is_number(const String& v)
{
  if (!is_number(v))
  {
    throw String("значение должно быть целым числом");
  }
}

bool is_numeric(const String &s)
{
  double num;
  bool res = (std::istringstream(s.c_str()) >> num).eof();
  return res;
}

void chk_value_is_numeric(const String& v)
{
  if (!is_numeric(v))
  {
    throw String("значение должно быть вещественным числом");
  }
}

bool is_bool(const String& s)
{
  return (s == "true") || (s == "false");
}

void chk_value_is_bool(const String& v)
{
  if (!is_bool(v))
  {
    throw String("значение должно иметь тип bool");
  }
}

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
  p_prefs->init_key("wop", "wake on power \\- проснуться при возобновлении питания \\(bool\\)",
  #ifdef LILYGO_WATCH_2020_V2
    "true",
  #else
    "false",
  #endif
    [](const String& value) -> void
  {
    chk_value_is_bool(value);

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

  p_prefs->init_key("tr", "установка порога", "95", [](const String& value) -> void
  {
    chk_value_is_number(value);

    MED_THRESHOLD = atoi(value.c_str());
  });

  p_prefs->init_key("trdt", "определяет, за сколько пунктов до порога уменьшать громкость шума", "10", [](const String& value) -> void
  {
    chk_value_is_number(value);

    MED_PRE_TRESHOLD_DELTA = atoi(value.c_str());
  });

#ifdef SOUND
  p_prefs->init_key("mnl", "максимальная громкость шума \\(numeric\\)", "0.1", [](const String& value) -> void
  {
    chk_value_is_numeric(value);

    float old_lvl = TNoise::get_level();
    float old_mnl = TNoise::MAX_NOISE_LEVEL;
    TNoise::MAX_NOISE_LEVEL = atof(value.c_str());
    TNoise::set_level(old_mnl ? (TNoise::MAX_NOISE_LEVEL * old_lvl) / old_mnl : TNoise::MAX_NOISE_LEVEL);
  });
#endif
  p_prefs->init_key("bod", "blink on data \\- мигнуть при поступлении нового пакета от гарнитуры \\(bool\\)", "false", [](const String& value) -> void
  {
    chk_value_is_bool(value);

    is_blink_on_packets = (value == "true");
  });

  xCFSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xCFSemaphore);

  p_fdb = new TFormulaDB();

  if(p_fdb->is_empty())
  {
    Serial.println("Выполняем первичную инициализацию базы формул.");
    p_fdb->assign("diss", "150 * (gl + gm) / d");
  }

  p_prefs->init_key("f", "формула", "diss", [this](const String& value) -> void
  {
    String val = this->p_fdb->get_value(value);
    
    TCalcFormula *pcf = this->p_fdb->compile(val);
    update_calc_formula(pcf);
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

TMyApplication *p_app = NULL;
void setup()
{
  esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
  Serial.begin(115200);
  Serial.print("Setup: priority = ");
  Serial.println(uxTaskPriorityGet(NULL));

  p_app = new TMyApplication();
}

void loop()
{
  p_app->run();
}
