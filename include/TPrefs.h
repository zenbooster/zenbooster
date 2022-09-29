#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <map>
#include <functional>

namespace Prefs
{
  using namespace std;

  typedef function<bool(String)> TCbChangeFunction;

  struct TPrefValue
  {
    String value; // это чтобы бот мог показать значение настройки по запросу
    String desc;
    TCbChangeFunction cb_change;
  };

  class TPrefs
  {
    private:
      const String name;
      Preferences prefs;
      std::map<String, TPrefValue> data;

    public:
      TPrefs(const String name);

      bool init_key(String key, String desc, String defval, TCbChangeFunction cb_change);
      bool contains(const String key) const;
      String operator [](String key);
      bool set_value(const String key, const String value);
      bool reinit_value(const String key);
      String get_desc(void);
      String get_values(void);
  };
}
