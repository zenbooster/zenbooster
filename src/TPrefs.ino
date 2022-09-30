#include "TPrefs.h"

namespace Prefs
{
TPrefs::TPrefs(const String name):
  name(name)
{
  //
}

bool TPrefs::init_key(String key, String desc, String defval, TCbChangeFunction cb_change)
{
  bool res;
  TPrefValue &pv = data[key];
  pv.desc = desc;
  pv.cb_change = cb_change;

  prefs.begin(name.c_str(), false);
  if(prefs.isKey(key.c_str()))
  {
    String value = prefs.getString(key.c_str()).c_str();
    res = cb_change(value);
    if(res)
    {
      pv.value = value;
    }
  }
  else
  {
    res = cb_change(defval);
    if(res)
    {
      prefs.putString(key.c_str(), defval.c_str());
      pv.value = defval;
    }
  }
  prefs.end();

  return res;
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

bool TPrefs::set_value(const String key, const String value)
{
  bool res = false;

  TPrefValue &pv = data[key];

  do // fake loop
  {
    if(!pv.cb_change)
    {
      Serial.printf("bool TPrefs::set_value(\"%s\", \"%s\"): Ошибка! Не вызван метод init_key.\n", key, value);
      break;
    }
    res = pv.cb_change(value);

    if(res)
    {
      prefs.begin(name.c_str(), false);
      if(value != pv.value)
      {
        prefs.putString(key.c_str(), value.c_str());
        pv.value = value;
      }
      prefs.end();
    }
  } while(false);

  return res;
}

bool TPrefs::reinit_value(const String key)
{
  String val = (*this)[key];
  Serial.printf("HIT.1: key=%s; val=%s\n", key, val);
  return set_value(key, val);
  //return set_value(key, (*this)[key]);
}

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
