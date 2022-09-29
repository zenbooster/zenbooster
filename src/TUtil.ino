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
}