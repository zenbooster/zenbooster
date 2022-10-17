#include "TConf.h"
#include "TMyApplication.h"
#include "TButtonIllumination.h"
#include "TNoise.h"
#include "TCalcFormula.h"
#include "TUtil.h"

namespace Conf
{
using namespace MyApplication;
using namespace ButtonIllumination;
using namespace Noise;
using namespace Util;

TConf::TConf():
  p_prefs(new TPrefs(TMyApplication::DEVICE_NAME)),
  p_fdb(NULL)

{
  TMyApplication::xOptRcMutex = xSemaphoreCreateRecursiveMutex();

  p_prefs->init_key("wop", "wake on power \\- проснуться при возобновлении питания \\(bool\\)",
  #ifdef LILYGO_WATCH_2020_V2
    "true",
  #else
    "false",
  #endif
    [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(is_validate_only)
    {
      return;
    }

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

  p_prefs->init_key("hsbtmacaddr", "headset bluetooth MAC address \\- MAC адрес нейрогарнитуры, разделитель \\- ':'", "20:21:04:08:39:93", [](const String& value, bool is_validate_only) -> void
  {
    uint8_t addr[6];
    TUtil::mac_2_array(value, addr);
    
    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      memcpy(TBluetoothStuff::address, addr, 6);
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

  p_prefs->init_key("tr", "установка порога", "100", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_number(value);
    int v = atoi(value.c_str());
    TUtil::chk_value_is_positive(v);
    TUtil::chk_value_is_not_zero(v);

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMyApplication::threshold = v;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

  p_prefs->init_key("pretr", "предпорог \\- определяет, за сколько пунктов до порога уменьшать громкость шума", "60", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_number(value);
    int v = atoi(value.c_str());
    TUtil::chk_value_is_positive(v);

    if(v > TMyApplication::threshold)
    {
      throw String("должно выполняться условие: значение <= tr");
    }

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMyApplication::pre_threshold = v;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

#ifdef SOUND
  p_prefs->init_key("mnl", "максимальная громкость шума \\(numeric\\)", "0.6", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_numeric(value);
    float v = atof(value.c_str());
    TUtil::chk_value_is_positive(v);

    if(v > 100.0)
    {
      throw String("должно выполняться условие: значение <= 100.0");
    }

    if(!is_validate_only)
    {
      float old_lvl = TNoise::get_level();
      float old_mnl = TNoise::MAX_NOISE_LEVEL;
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TNoise::MAX_NOISE_LEVEL = v;
      TNoise::set_level(old_mnl ? (TNoise::MAX_NOISE_LEVEL * old_lvl) / old_mnl : TNoise::MAX_NOISE_LEVEL);
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });
#endif

#ifdef PIN_BTN
  p_prefs->init_key("mil", "максимальный уровень подсветки \\(numeric\\)", "75.0", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_numeric(value);
    float v = atof(value.c_str());
    TUtil::chk_value_is_positive(v);

    if(v > 100.0)
    {
      throw String("должно выполняться условие: значение <= 100.0");
    }

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TButtonIllumination::max_illumination_level = v;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    
      if(TMyApplication::p_btn_il && !TBluetoothStuff::is_connected)
      {
        TButtonIllumination::on_wait_for_connect();
      }
    }
  });
#endif

  p_prefs->init_key("bod", "blink on data \\- мигнуть при поступлении нового пакета от гарнитуры \\(bool\\)", "false", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(!is_validate_only)
    {
      TButtonIllumination::is_blink_on_packets = (value == "true");
    }
  });

  p_fdb = new TFormulaDB();

  if(p_fdb->is_empty())
  {
    Serial.println("Выполняем первичную инициализацию базы формул.");
    // https://scriptures.ru/yoga/eeg_voprosy_i_otvety.htm#formula2
    p_fdb->assign("samadhi", "(ah*0.8 + al) * 75 / (bh + bl + t*0.4 + d*0.08) - 10");
    p_fdb->assign("gamma", "100 * 3 * gl / (gm + bh + bl)");
  }

  p_prefs->init_key("f", "формула", "diss", [this](const String& value, bool is_validate_only) -> void
  {
    String val = p_fdb->get_value(value);
    
    TCalcFormula *pcf = TCalcFormula::compile(val);

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMyApplication::update_calc_formula(pcf);
      TMedSession::formula_name = value;
      TMedSession::formula_text = val;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

  p_prefs->init_key("ld", "log data \\- отправлять уровни ритмов, приходящих от гарнитуры \\(bool\\)", "false", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMyApplication::is_log_data_to_bot = (value == "true");
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

  p_prefs->init_key("ups", "use poor signal \\- использовать признак POOR\\_SIGNAL от гарнитуры \\(bool\\)", "true", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMyApplication::is_use_poor_signal = (value == "true");
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });

  p_prefs->init_key("minsessec", "минимальная продолжительность сессии медитации в секундах, для формирования отчёта", "15", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_number(value);
    int v = atoi(value.c_str());
    TUtil::chk_value_is_positive(v);
    TUtil::chk_value_is_not_zero(v);
    
    if(!is_validate_only)
    {
      xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
      TMedSession::minsessec = v;
      xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
    }
  });
}

TConf::~TConf()
{
  if(p_fdb)
    delete p_fdb;
  if(p_prefs)
    delete p_prefs;
}

TPrefs *TConf::get_prefs()
{
  return p_prefs;
}

TFormulaDB *TConf::get_fdb()
{
  return p_fdb;
}

DynamicJsonDocument TConf::get_json(void)
{
  DynamicJsonDocument doc(1024);

  doc[key_options] = p_prefs->get_json();
  doc[key_formulas] = p_fdb->get_json();

  return doc;
}

void TConf::validate_json(const DynamicJsonDocument& doc)
{
  if(doc.size() != 2)
  {
    throw String("корень JSON должен состоять из 2-х элементов");
  }

  function<void(const char *)> fn = [doc](const char *key) -> void
  {
    if(!doc.containsKey(key))
    {
      throw String("корень JSON не содержит ключа \"") + key + "\"";
    }
  };
  const char *key_o = key_options;
  fn(key_o);
  const char *key_f = key_formulas;
  fn(key_f);

  String f_val = doc[key_o]["f"];
  if(!doc[key_f].containsKey(f_val))
  {
    throw "список формул не содержит ключа \"" + f_val + "\" из \"f\"";
  }

  p_prefs->validate_json(doc[key_o]);
  p_fdb->validate_json(doc[key_f]);
}

void TConf::add_json(const DynamicJsonDocument& doc)
{
  validate_json(doc);
  p_fdb->add_json(doc[key_formulas]);
  p_prefs->set_json(doc[key_options]);
}

void TConf::set_json(const DynamicJsonDocument& doc)
{
  validate_json(doc);
  p_fdb->set_json(doc[key_formulas]);
  p_prefs->set_json(doc[key_options]);
}
}