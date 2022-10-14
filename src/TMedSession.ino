#include "TMedSession.h"

namespace MedSession
{
TMedSession::TMedSession(uint32_t threshold):
    threshold(threshold),
    sess_time_sec(0),
    med_tot_time_sec(0),
    med_sd_time_sec(0),
    med_msd_time_sec(0),
    avg_med_val(0)
{
    //
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

    avg_med_val += (med - avg_med_val) / sess_time_sec;
}

String TMedSession::gen_report(void) const
{
    String res = 
        "Продолжительность сессии: " + String(sess_time_sec) + " с.\n"
        "Общая продолжительность медитации: " + String(med_tot_time_sec) + " с.\n"
        "Максимальная продолжительность непрерывной медитации: " + String(med_msd_time_sec) + " с.\n"
        "Среднее значение уровня медитации: " + String(avg_med_val, 1);

    return res;
}
}