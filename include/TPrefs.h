#pragma once
#include <Preferences.h>
#include <map>
#include <string>
#include <functional>

namespace Prefs
{
  using namespace std;

  typedef function<bool(string)> TCbChangeFunction;

  struct TPrefValue
  {
    string value; // это чтобы бот мог показать значение настройки по запросу
    string desc;
    TCbChangeFunction cb_change;
  };

  class TPrefs
  {
    private:
      const string name;
      Preferences prefs;
      std::map<string, TPrefValue> data;

    public:
      TPrefs(const string name);

      bool init_key(string key, string desc, string defval, TCbChangeFunction cb_change);
      bool contains(const string key) const;
      string operator [](string key);
      bool set_value(const string key, const string value);
      bool reinit_value(const string key);
      string get_desc(void);
      string get_values(void);
  };
}
