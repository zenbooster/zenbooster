#include "TPrefs.h"

namespace Prefs
{
TPrefs::TPrefs(const String name):
  name(name)
{
  //
}

void TPrefs::init_key(String key, String desc, String defval, TCbChangeFunction cb_change)
{
  TPrefValue &pv = data[key];
  pv.desc = desc;
  pv.cb_change = cb_change;

  prefs.begin(name.c_str(), false);
  if(prefs.isKey(key.c_str()))
  {
    String value = prefs.getString(key.c_str()).c_str();
    cb_change(value);

    pv.value = value;
  }
  else
  {
    cb_change(defval);

    prefs.putString(key.c_str(), defval.c_str());
    pv.value = defval;
  }
  prefs.end();
}

bool TPrefs::contains(const String key) const
{
  //return data.contains(key);
  return data.count(key);
}

String TPrefs::operator [](String key)
{
  return data[key].value;
}

void TPrefs::set_value(const String key, const String value)
{
  TPrefValue &pv = data[key];

  do // fake loop
  {
    if(!pv.cb_change)
    {
      Serial.printf("bool TPrefs::set_value(\"%s\", \"%s\"): Ошибка! Не вызван метод init_key.\n", key.c_str(), value.c_str());
      break;
    }
    pv.cb_change(value);

    prefs.begin(name.c_str(), false);
    if(value != pv.value)
    {
      prefs.putString(key.c_str(), value.c_str());
      pv.value = value;
    }
    prefs.end();
  } while(false);
}

/*void TPrefs::reinit_value(const String key)
{
  set_value(key, (*this)[key]);
}*/

String TPrefs::get_desc(void)
{
  String res;

  for(std::map<String, TPrefValue>::iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    String k =  iter->first;
    TPrefValue v = iter->second;

    String line = "*" + k + "* \\- " + v.desc + "\n";
    res += line;
  }
  return res;
}

String TPrefs::get_values(void)
{
  String res;

  for(std::map<String, TPrefValue>::iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    String k =  iter->first;
    TPrefValue v = iter->second;
    String s = v.value;

    String line = "*" + k + "* \\= `" + s + "`\n";
    res += line;
  }
  return res;
}
}
