#include <time.h>
#include <esp_coexist.h>
#include <WiFiManager.h>
#include "TBluetoothStuff.h"
#include "TWiFiStuff.h"
#include "TPrefs.h"
#include <string>
#include <exception>
#include "TNoise.h"
#include "expression.h"
#include "parser.h"
#include "lexer.h"

using namespace std;
using namespace Noise;
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace Prefs;

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

int yyparse(SExpression **expression, yyscan_t scanner);

SExpression *getAST(const char *expr)
{
    SExpression *expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    if (yylex_init(&scanner)) {
        /* could not initialize */
        return NULL;
    }

    state = yy_scan_string(expr, scanner);

    if (yyparse(&expression, scanner)) {
        /* error parsing */
        return NULL;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return expression;
}

float evaluate(SExpression *e)
{
    switch (e->type) {
        case eVAL:
            return e->val;
        case eVAR:
            return (float)*e->p_var;
        case eDIV:
            return evaluate(e->left) / evaluate(e->right);
        case eMUL:
            return evaluate(e->left) * evaluate(e->right);
        case eSUB:
            return evaluate(e->left) - evaluate(e->right);
        case eADD:
            return evaluate(e->left) + evaluate(e->right);
        default:
            /* should not be here */
            return 0;
    }
}

//const int LED_BUILTIN = 2;

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

struct TRingBufferInItem
{
  time_t time;
  int delta;
  int theta;
  int alpha_lo;
  int alpha_hi;
  int beta_lo;
  int beta_hi;
  int gamma_lo;
  int gamma_md;
};

class TCalcFormula: public TRingBufferInItem
{
  private:
    SExpression *p_ast;

  public:
    TCalcFormula(string ex);
    ~TCalcFormula()
    {
      deleteExpression(p_ast);
      dict_name_id.clear();
      dict_id_ptr.clear();
    }

    float run(void)
    {
      return evaluate(p_ast);
    }
};

TCalcFormula::TCalcFormula(string ex)
{
  reg_var("d", &delta);
  reg_var("t", &theta);
  reg_var("al", &alpha_lo);
  reg_var("ah", &alpha_hi);
  reg_var("bl", &beta_lo);
  reg_var("bh", &beta_hi);
  reg_var("gl", &gamma_lo);
  reg_var("gm", &gamma_md);
  p_ast = getAST(ex.c_str());

  if(!p_ast)
  {
    throw string("Не могу скомпилировать формулу!");
  }
}

struct TRingBufferOutItem
{
  time_t time;
  int meditation;
};

class TMyApplication
{
  private:
    const char *DEVICE_NAME = "zenbooster-dev";
    const char *WIFI_SSID = DEVICE_NAME;
    const char *WIFI_PASS = "zbdzbdzbd";
    
    WiFiManager wifiManager;

    static int MED_THRESHOLD;// = 95;
    static int MED_PRE_TRESHOLD_DELTA;// = 10;

    TRingBufferInItem ring_buffer_in[4];
    int ring_buffer_in_index;
    int ring_buffer_in_size;

    // размер подобрать под ширину отображаемого графика:
    TRingBufferOutItem ring_buffer_out[1024];
    int ring_buffer_out_index;
    
    TNoise *p_noise;
    TBluetoothStuff *p_bluetooth_stuff;
    TWiFiStuff *p_wifi_stuff;
    TPrefs *p_prefs;
    static TCalcFormula *p_calc_formula;
    static SemaphoreHandle_t xCFSemaphore;

    int calc_formula_meditation();
    static int int_from_12bit(unsigned char *buf);
    static void callback(unsigned char code, unsigned char *data, void *arg);

  public:
    TMyApplication();
    ~TMyApplication();
    void run(void);
  
};

int TMyApplication::MED_THRESHOLD;
int TMyApplication::MED_PRE_TRESHOLD_DELTA;
TCalcFormula *TMyApplication::p_calc_formula = NULL;
SemaphoreHandle_t TMyApplication::xCFSemaphore;

int TMyApplication::calc_formula_meditation()
{
  double res = 0;

  for(int i = 0; i < ring_buffer_in_size; i++)
  {
    TRingBufferInItem *p_item = ring_buffer_in + ((ring_buffer_in_index - i) & 3);
    xSemaphoreTake(xCFSemaphore, portMAX_DELAY);
    if (!p_calc_formula)
    {
      xSemaphoreGive(xCFSemaphore);
      throw string("Объект формулы (TCalcFormula) равен NULL!");
    }

    TRingBufferInItem *pcf = p_calc_formula;
    *pcf = *p_item; // копируем уровни ритмов
    res += p_calc_formula->run();
    xSemaphoreGive(xCFSemaphore);
  }
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
    
    TRingBufferOutItem item = {now, med};
    p_this->ring_buffer_out[p_this->ring_buffer_out_index] = item;
    p_this->ring_buffer_out_index = (p_this->ring_buffer_out_index + 1) & 3;

    SerialPrintf("delta=%d, theta=%d, alpha_lo=%d, alpha_hi=%d, beta_lo=%d, beta_hi=%d, gamma_lo=%d, gamma_md=%d; --> f_med=%d\n", delta, theta, alpha_lo, alpha_hi, beta_lo, beta_hi, gamma_lo, gamma_md, med);

    if(med > TMyApplication::MED_THRESHOLD)
    {
      TNoise::set_level(0);
    }
    else
    {
      int d = TMyApplication::MED_THRESHOLD - med;

      if(d < TMyApplication::MED_PRE_TRESHOLD_DELTA)
      {
        TNoise::set_level((d * TNoise::MAX_NOISE_LEVEL) / TMyApplication::MED_PRE_TRESHOLD_DELTA);
      }
      else
      {
        TNoise::set_level(TNoise::MAX_NOISE_LEVEL);
      }
    }
  } // if (code == 0x83)
} // void TMyApplication::callback(unsigned char code, unsigned char *data, void *arg)

bool is_number(const std::string &s)
{
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

TMyApplication::TMyApplication():
  ring_buffer_in({}),
  ring_buffer_in_index(0),
  ring_buffer_in_size(0),
  ring_buffer_out({}),
  ring_buffer_out_index(0),
  p_noise(NULL),
  p_prefs(NULL)
{
  //Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  // инициализация WiFi:
  // Временно запретим wifi, т.к. есть проблемы сосуществования wifi и bluetooth на одном радио.
  // Ждём модуль bluetooth HC-06 - он должен рещить все проблемы.
  wifiManager.setHostname(DEVICE_NAME);
  wifiManager.autoConnect(WIFI_SSID, WIFI_PASS);

  p_prefs = new TPrefs(DEVICE_NAME);

  p_prefs->init_key("tr", "установка порога", "95", [](string value) -> bool {
    bool res = is_number(value);

    if(res)
    {
      MED_THRESHOLD = atoi(value.c_str());
    }
    
    return res;
  });

  p_prefs->init_key("trdt", "определяет, за сколько пунктов до порога уменьшать громкость шума", "10", [](string value) -> bool {
    bool res = is_number(value);

    if(res)
    {
      MED_PRE_TRESHOLD_DELTA = atoi(value.c_str());
    }
    
    return res;
  });

  p_prefs->init_key("mnl", "максимальная громкость шума", "5", [](string value) -> bool {
    bool res = is_number(value);

    if(res)
    {
      int old_lvl = TNoise::get_level();
      int old_mnl = TNoise::MAX_NOISE_LEVEL;
      TNoise::MAX_NOISE_LEVEL = atoi(value.c_str());
      TNoise::set_level(old_mnl ? (TNoise::MAX_NOISE_LEVEL * old_lvl) / old_mnl : 0);
    }
    
    return res;
  });

  xCFSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xCFSemaphore);

  p_prefs->init_key("f", "формула", "gl/50", [](string value) -> bool {
    xSemaphoreTake(xCFSemaphore, portMAX_DELAY);
    TCalcFormula *pcf;
    try
    {
      pcf = new TCalcFormula(value);
    }
    catch(string e)
    {
      Serial.println(e.c_str());
    }

    if (pcf)
    {
      if (p_calc_formula)
      {
        delete p_calc_formula;
      }
      p_calc_formula = pcf;
    }

    bool res = pcf;
    xSemaphoreGive(xCFSemaphore);

    return res;
  });

  p_wifi_stuff = new TWiFiStuff(DEVICE_NAME, p_prefs);
  p_bluetooth_stuff = new TBluetoothStuff(DEVICE_NAME, this, callback);
  //p_wifi_stuff = new TWiFiStuff(DEVICE_NAME, p_prefs);
  p_noise = new TNoise();
}

TMyApplication::~TMyApplication()
{
  if(p_prefs)
    delete p_prefs;
  if(p_bluetooth_stuff)
    delete p_bluetooth_stuff;
  if(p_wifi_stuff)
    delete p_wifi_stuff;
  if(p_noise)
    delete p_noise;
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
