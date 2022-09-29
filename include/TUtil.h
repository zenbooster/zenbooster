#pragma once
#include <Arduino.h>

namespace Util
{

class TUtil
{
    private:
        static void parse_bytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

    public:
        static String screen_mark_down(const String s);
        static bool mac_2_array(String mac, uint8_t *buf);
};
}