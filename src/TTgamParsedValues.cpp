#include "TTgamParsedValues.h"

namespace TgamParsedValues
{
    void TTgamParsedValues::add_string(String& dst, const String& src)
    {
        if(!dst.isEmpty())
        {
            dst += ", ";
        }
        dst += src;
    }

    const String TTgamParsedValues::serialize(void) const
    {
        String res;

        if(is_has_batt)
        {
            add_string(res, "batt=" + String(batt));
        }

        if (is_has_poor)
        {
            add_string(res, "poor=" + String(poor));
        }

        if (is_has_eeg_power)
        {
            add_string(res,
                "d="+String(delta)+", t="+String(theta)+
                ", al="+String(alpha_lo)+", ah="+String(alpha_hi)+
                ", bl="+String(beta_lo)+", bh="+String(beta_hi)+
                ", gl="+String(gamma_lo)+", gm="+String(gamma_md)+
                ", em="+String(esense_med)+", ea="+String(esense_att)
            );
        }

        return res;
    }

    DynamicJsonDocument TTgamParsedValues::get_json(void)
    {
        DynamicJsonDocument res(256);

        res["time"] = time;

        if(is_has_batt)
        {
            res["batt"] = batt;
        }

        if(is_has_poor)
        {
            res["poor"] = poor;
        }

        if(is_has_eeg_power)
        {
            res["d"] = delta;
            res["t"] = theta;
            res["al"] = alpha_lo;
            res["ah"] = alpha_hi;
            res["bl"] = beta_lo;
            res["bh"] = beta_hi;
            res["gl"] = gamma_lo;
            res["gm"] = gamma_md;
            res["em"] = esense_med;
            res["ea"] = esense_att;
        }

        return res;
    }
}