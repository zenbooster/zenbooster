#include "TMedSession.h"
#include "TUtil.h"
#include "TWiFiStuff.h"
#include "TConf.h"

namespace MedSession
{
using namespace Util;

uint32_t TMedSession::threshold;
uint32_t TMedSession::pre_threshold;
uint16_t TMedSession::minsessec;
String TMedSession::formula_name;
String TMedSession::formula_text;

TMedSession::TMedSession(TWiFiStuff *p_wifi_stuff):
    p_wifi_stuff(p_wifi_stuff),
    sess_time_sec(0),
    med_tot_time_sec(0),
    med_sd_time_sec(0),
    med_msd_time_sec(0),
    max_med_val(0),
    avg_med_val(0)
{
    //
}

TMedSession::~TMedSession()
{
    if(sess_time_sec >= minsessec)
    {
        p_wifi_stuff->tgb_send(
            "*Отчёт по сессии:*\n`" + 
            TUtil::screen_mark_down(
            "Формула: " + formula_name + " = " + formula_text + "\n" +
            "Порог: " + String(threshold) + "\n" +
            "Предпорог: " + String(pre_threshold) + "\n" +
            gen_report()
            ) + 
            "`"
        );
    }
}

void TMedSession::calc_next(int32_t med)
{
    // данные приходят раз в секунду, по этому обойдёмся простым инкрементом:
    sess_time_sec++;
    if(med >= threshold)
    {
        med_tot_time_sec++;
        med_sd_time_sec++;

        if(med_sd_time_sec > med_msd_time_sec)
        {
            med_msd_time_sec = med_sd_time_sec;
        }
    }
    else
    {
        med_sd_time_sec = 0;
    }

    if(med > max_med_val)
    {
        max_med_val = med;
    }

    avg_med_val += (med - avg_med_val) / sess_time_sec;
}

String TMedSession::gen_report(void) const
{
    String res = 
        "Продолжительность сессии: " + String(sess_time_sec) + " с.\n"
        "Общая продолжительность медитации (ОПМ): " + String(med_tot_time_sec) + " с. (" + String((med_tot_time_sec * 100) / sess_time_sec) + "%)\n"
        "Максимальная продолжительность непрерывной медитации: " + String(med_msd_time_sec) + " с. (" + String(med_tot_time_sec ? (med_msd_time_sec * 100) / med_tot_time_sec : 0) + "% ОПМ)\n"
        "Максимальное значение уровня медитации: " + String(max_med_val);
        "Среднее значение уровня медитации: " + String(avg_med_val, 1);

    return res;
}
}