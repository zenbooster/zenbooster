#pragma once
#include <Arduino.h>

namespace Util
{

class TUtil
{
    private:
        static bool is_number(const String &s);
        static bool is_numeric(const String &s);
        static bool is_bool(const String& s);

        static void parse_bytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

    public:
        static void chk_value_is_number(const String& v);
        static void chk_value_is_numeric(const String& v);
        static void chk_value_is_bool(const String& v);

        static String screen_mark_down(const String s);
        static bool mac_2_array(String mac, uint8_t *buf);
};
}