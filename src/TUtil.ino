#include "TUtil.h"
#include <sstream>

namespace Util
{
using namespace std;

bool TUtil::is_number(const String &s)
{
  return !s.isEmpty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void TUtil::chk_value_is_number(const String& v)
{
  if (!is_number(v))
  {
    throw String("значение должно быть целым числом");
  }
}

bool TUtil::is_numeric(const String &s)
{
  double num;
  bool res = (std::istringstream(s.c_str()) >> num).eof();
  return res;
}

void TUtil::chk_value_is_numeric(const String& v)
{
  if (!is_numeric(v))
  {
    throw String("значение должно быть вещественным числом");
  }
}

bool TUtil::is_bool(const String& s)
{
  return (s == "true") || (s == "false");
}

void TUtil::chk_value_is_bool(const String& v)
{
  if (!is_bool(v))
  {
    throw String("значение должно иметь тип bool");
  }
}

template<class T>
void TUtil::chk_value_is_positive(T v)
{
  if(v < 0)
  {
    throw String("значение должно быть положительным");
  }
}

template<class T>
void TUtil::chk_value_is_not_zero(T v)
{
  if(!v)
  {
    throw String("значение не может быть равным нулю");
  }
}

String TUtil::screen_mark_down(const String s)
{
    String res = s;
    char c2r[] = "\\`~!@#$%^&*()-_=+[{]}|;:'\",<.>/?";

    char *p = c2r;
    char *p_end = p + sizeof(c2r);
    for(; p < p_end;)
    {
        char c = *p++;

        if(res.indexOf(c) > -1)
        {
            char src[] = " ";
            char dst[] = "\\ ";

            src[0] = c;
            dst[1] = c;
            res.replace(src, dst);
        }
    }

    return res;
}

// take a hex string and convert it to a 32bit number (max 8 hex digits)
uint32_t TUtil::hex2int(const char *hex, int len)
{
  uint32_t val = 0;
  while (*hex && len--)
  {
    // get current character then increment
    uint8_t byte = *hex++; 
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (byte >= '0' && byte <= '9') byte = byte - '0';
    else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
    else
    {
      throw "hex2int(..): символ '" + String((char)byte) + "' не является цифрой";
    }
    // shift 4 to make space for new digit, and add the 4 bits of the new digit 
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}

void TUtil::parse_bytes(const char* str, char sep, byte* bytes, int maxBytes, /*int base,*/ function<void(const char *p0, const char *p1)> cb)
{
  const char *p;
  int i = 0;

  for( ; i < maxBytes; i++)
  {
    p = strchr(str, sep);

    if(cb)
    {
      cb(str, p);
    }

    //bytes[i] = strtoul(str, NULL, base);
    bytes[i] = hex2int(str, 2);
    str = p;
    if (str == NULL || *str == '\0')
    {
      break;
    }
    str++;
  }
}

void TUtil::mac_2_array(String mac, uint8_t *buf)
{
  int i = 0;

  parse_bytes(mac.c_str(), ':', buf, 6, //0x10,
    [&i](const char *p0, const char *p1) -> void
    {
      i++;
      if(p1 && (p1 - p0 != 2))
      {
        throw String("неверный MAC адрес, байт должен состоять из 2-х цифр");
      }
    }
  );

  if(i != 6)
  {
    throw String("MAC адрес должен состоять из 6-ти байт");
  }
}

template<class T>
T TUtil::percent_of(float pct, T val)
{
  return (pct * 100.0) / (float)val;
}

void TUtil::chk_nvs_key(const String& key)
{
    if(key.length() > 15)
    {
        throw String("void TUtil::chk_nvs_key(..): длина ключа должна быть меньше либо равна 15 символам");
    }
}
}