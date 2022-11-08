#pragma once
#include <Arduino.h>
#include <functional>
#include <memory>

namespace Util
{
using namespace std;

class TUtil
{
    private:
        static bool is_number(const String &s);
        static bool is_numeric(const String &s);
        static bool is_bool(const String& s);

        static uint32_t hex2int(const char *hex, int len=8);
        static void parse_bytes(const char* str, char sep, byte* bytes, int maxBytes, /*int base,*/ function<void(const char *p0, const char *p1)> cb = NULL);

    public:
        static void chk_value_is_number(const String& v);
        static void chk_value_is_numeric(const String& v);
        static void chk_value_is_bool(const String& v);

        template<class T>
        static void chk_value_is_positive(T v);
        template<class T>
        static void chk_value_is_not_zero(T v);
        template<class T>
        static T percent_of(float pct, T val);

        static String screen_mark_down(const String& s);
        static shared_ptr<char> screen_mark_down(const char *s);
        static inline shared_ptr<char> screen_mark_down(const shared_ptr<char> s)
        {
          return screen_mark_down(s.get());
        }
        static void mac_2_array(String mac, uint8_t *buf);

        static void chk_nvs_key(const String& key); // может бросить исключение

        template <class ... Args>
        static shared_ptr<char> sprintf(Args ... args)
        {
          char *buf = 0;
          const size_t sz = snprintf(buf, 0, args...) + 1;
          //if (nBufferLength == 1) return 0;
          buf = new char[sz];
          //if(!buf) return -sz;
          snprintf(buf, sz, args...);
          return shared_ptr<char>(buf);
        }
};

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

template<class T>
T TUtil::percent_of(float pct, T val)
{
  return (pct * (float)val) / 100.0;
}
}