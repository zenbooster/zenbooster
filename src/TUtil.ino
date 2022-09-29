#include "TUtil.h"

namespace Util
{
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

void TUtil::parse_bytes(const char* str, char sep, byte* bytes, int maxBytes, int base)
{
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

bool TUtil::mac_2_array(String mac, uint8_t *buf)
{
    parse_bytes(mac.c_str(), ':', buf, 6, 0x10);
    return true;    
}
}