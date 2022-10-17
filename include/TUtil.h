#pragma once
#include <Arduino.h>
#include <functional>

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

        static String screen_mark_down(const String s);
        static void mac_2_array(String mac, uint8_t *buf);

        static void chk_nvs_key(const String& key); // может бросить исключение
};
}