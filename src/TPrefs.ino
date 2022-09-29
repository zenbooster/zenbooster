#include "TPrefs.h"

namespace Prefs
{
TPrefs::TPrefs(const string name):
  name(name)
{
  //
}

bool TPrefs::init_key(string key, string desc, string defval, TCbChangeFunction cb_change)
{
  bool res;
  TPrefValue &pv = data[key];
  pv.desc = desc;
  pv.cb_change = cb_change;

  prefs.begin(name.c_str(), false);
  if(prefs.isKey(key.c_str()))
  {
    string value = prefs.getString(key.c_str()).c_str();
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

bool TPrefs::contains(const string key) const
{
  //return data.contains(key);
  return data.count(key);
}

string TPrefs::operator [](string key)
{
  return data[key].value;
}

bool TPrefs::set_value(const string key, const string value)
{
  bool res = true;

  TPrefValue &pv = data[key];

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

  return res;
}

bool TPrefs::reinit_value(const string key)
{
  return set_value(key, (*this)[key]);
}

string TPrefs::get_desc(void)
{
  string res;

  for(std::map<string, TPrefValue>::iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    string k =  iter->first;
    TPrefValue v = iter->second;

    string line = "*" + k + "* \\- " + v.desc + "\n";
    res += line;
  }
  return res;
}

string TPrefs::get_values(void)
{
  string res;

  for(std::map<string, TPrefValue>::iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    string k =  iter->first;
    TPrefValue v = iter->second;
    string s = v.value;

    string line = "*" + k + "* \\= `" + s + "`\n";
    res += line;
  }
  return res;
}
}
