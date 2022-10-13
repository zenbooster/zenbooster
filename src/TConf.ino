#include "TConf.h"
#include "TMyApplication.h"
#include "TNoise.h"
#include "TCalcFormula.h"
#include "TUtil.h"

namespace Conf
{
using namespace Noise;
using namespace Util;

TConf::TConf(TMyApplication *p_app):
  p_prefs(new TPrefs(TMyApplication::DEVICE_NAME)),
  p_fdb(NULL)

{
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

  p_prefs->init_key("tr", "установка порога", "100", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_number(value);

    if(!is_validate_only)
    {
      TMyApplication::MED_THRESHOLD = atoi(value.c_str());
    }
  });

  p_prefs->init_key("trdt", "определяет, за сколько пунктов до порога уменьшать громкость шума", "60", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_number(value);

    if(!is_validate_only)
    {
      TMyApplication::MED_PRE_TRESHOLD_DELTA = atoi(value.c_str());
    }
  });

#ifdef SOUND
  p_prefs->init_key("mnl", "максимальная громкость шума \\(numeric\\)", "0.6", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_numeric(value);

    if(!is_validate_only)
    {
      float old_lvl = TNoise::get_level();
      float old_mnl = TNoise::MAX_NOISE_LEVEL;
      TNoise::MAX_NOISE_LEVEL = atof(value.c_str());
      TNoise::set_level(old_mnl ? (TNoise::MAX_NOISE_LEVEL * old_lvl) / old_mnl : TNoise::MAX_NOISE_LEVEL);
    }
  });
#endif
  p_prefs->init_key("bod", "blink on data \\- мигнуть при поступлении нового пакета от гарнитуры \\(bool\\)", "false", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(is_validate_only)
    {
      TMyApplication::is_blink_on_packets = (value == "true");
    }
  });

  TMyApplication::xCFSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(TMyApplication::xCFSemaphore);

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
      TMyApplication::update_calc_formula(pcf);
    }
  });

  p_prefs->init_key("ld", "log data \\- отправлять уровни ритмов, приходящих от гарнитуры \\(bool\\)", "false", [](const String& value, bool is_validate_only) -> void
  {
    TUtil::chk_value_is_bool(value);

    if(!is_validate_only)
    {
      TMyApplication::is_log_data_to_bot = (value == "true");
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

  doc["options"] = p_prefs->get_json();
  doc["formulas"] = p_fdb->get_json();

  return doc;
}

void TConf::validate_json(DynamicJsonDocument& doc)
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
  const char *key_o = "options";
  fn(key_o);
  const char *key_f = "formulas";
  fn(key_f);

  //const DynamicJsonDocument& dc = doc[key_o];
  p_prefs->validate_json(doc[key_o]);
  p_fdb->validate_json(doc[key_f]);
}
}