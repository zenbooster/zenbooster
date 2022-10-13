#include "TPrefs.h"
#include "TUtil.h"

namespace Prefs
{
using namespace Util;

TPrefs::TPrefs(const String& name):
  name(name)
{
  //
}

void TPrefs::init_key(const String& key, const String& desc, const String& defval, TCbChangeFunction cb_change)
{
  TPrefValue &pv = data[key];
  pv.desc = desc;
  pv.cb_change = cb_change;

  prefs.begin(name.c_str(), false);
  if(prefs.isKey(key.c_str()))
  {
    String value = prefs.getString(key.c_str()).c_str();
    cb_change(value, false);

    pv.value = value;
  }
  else
  {
    cb_change(defval, false);

    prefs.putString(key.c_str(), defval.c_str());
    pv.value = defval;
  }
  prefs.end();
}

bool TPrefs::contains(const String& key) const
{
  //return data.contains(key);
  return data.count(key);
}

String TPrefs::operator [](const String& key)
{
  return data[key].value;
}

void TPrefs::set_value(const String& key, const String& value)
{
  TPrefValue &pv = data[key];

  do // fake loop
  {
    if(!pv.cb_change)
    {
      Serial.printf("bool TPrefs::set_value(\"%s\", \"%s\"): Ошибка! Не вызван метод init_key.\n", key.c_str(), value.c_str());
      break;
    }
    pv.cb_change(value, false);

    prefs.begin(name.c_str(), false);
    if(value != pv.value)
    {
      prefs.putString(key.c_str(), value.c_str());
      pv.value = value;
    }
    prefs.end();
  } while(false);
}

/*void TPrefs::reinit_value(const String& key)
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

DynamicJsonDocument TPrefs::get_json(void)
{
  DynamicJsonDocument res(1024);

  for(std::map<String, TPrefValue>::iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    String k =  iter->first;
    TPrefValue v = iter->second;
    String s = v.value;

    res[k] = s;
  }
  return res;
}

void TPrefs::validate_json(const DynamicJsonDocument& doc)
{
  if(doc.size() != data.size())
  {
    throw String("не совпадает количество ключей");
  }

  JsonObjectConst root = doc.as<JsonObjectConst>();
  for (JsonPairConst kv : root)
  {
    String key = kv.key().c_str();
    TUtil::chk_nvs_key(key);

    if(!data.count(key))
    {
      throw String("не существует опции с именем \"" + key + "\"");
    }

    String val = kv.value().as<const char *>();
    if(val.isEmpty())
    {
        throw "значение для \"" + key + "\" не должно быть пустым";
    }
    
    try
    {
      data[key].cb_change(val, true);
    }
    catch(String& e)
    {
      throw key + ": " + e;
    }
  }
}

void TPrefs::set_json(const DynamicJsonDocument& doc)
{
  validate_json(doc);

  JsonObjectConst root = doc.as<JsonObjectConst>();
  for (JsonPairConst kv : root)
  {
    Serial.printf("TPrefs::set_json(..): set_value(\"%s\", \"%s\")\n", kv.key().c_str(), kv.value().as<const char *>());
    /*set_value(
        kv.key().c_str(),
        kv.value().as<const char *>()
    );*/
  }
}
}