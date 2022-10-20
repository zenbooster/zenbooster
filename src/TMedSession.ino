#include "TMedSession.h"
#include "TMyApplication.h"
#include "TUtil.h"
#include "TWiFiStuff.h"
#include "TConf.h"

namespace MedSession
{
using namespace Util;
using namespace MyApplication;

uint16_t TMedSession::minsessec;
String TMedSession::formula_name;
String TMedSession::formula_text;

TMedSession::TMedSession():
    sess_time_sec(0),
    med_tot_time_sec(0),
    med_sd_time_sec(0),
    med_sd_count(0),
    med_msd_time_sec(0),
    med_asd_time_sec(0),
    max_med_val(0),
    avg_med_val(0)
{
    sess_beg = TWiFiStuff::time_cli.getEpochTime();
    // по хорошему, если порог или предпорог поменялись пока сессия была открыта,
    // надо её закрыть, применить изменения порогов и затем снова открыть сессию...
    xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
    tr = TMyApplication::threshold;
    pretr = TMyApplication::pre_threshold;
    xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
}

TMedSession::~TMedSession()
{
    xSemaphoreTakeRecursive(TMyApplication::xOptRcMutex, portMAX_DELAY);
    uint16_t mss = minsessec;

    if(sess_time_sec >= mss)
    {
        TWiFiStuff::tgb_send(
            "*Отчёт по сессии:*\n`" + 
            TUtil::screen_mark_down(
            "Формула: " + formula_name + " = " + formula_text + "\n"
            "Порог: " + String(tr) + "\n"
            "Предпорог: " + String(pretr) + "\n" +
            gen_report()
            ) + 
            "`"
        );
    }
    xSemaphoreGiveRecursive(TMyApplication::xOptRcMutex);
}

void TMedSession::calc_next(int32_t med)
{
    // данные приходят раз в секунду, по этому обойдёмся простым инкрементом:
    sess_time_sec++;
    if(med >= tr)
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
        if(med_sd_time_sec)
        {
            med_sd_count++;
            med_asd_time_sec += (med_sd_time_sec - med_asd_time_sec) / med_sd_count;
            med_sd_time_sec = 0;
        }
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
        "Начало сессии: " + TWiFiStuff::time_cli.getFormattedDate(sess_beg) + "\n"
        "Продолжительность сессии: " + String(sess_time_sec) + " с.\n"
        "Общая продолжительность медитации (ОПМ): " + String(med_tot_time_sec) + " с. (" + String((med_tot_time_sec * 100) / sess_time_sec) + "%)\n"
        "Общее количество непрерывных медитаций: " + String(med_sd_count) + "\n"
        "Максимальная продолжительность непрерывной медитации: " + String(med_msd_time_sec) + " с. (" + String(med_tot_time_sec ? (med_msd_time_sec * 100) / med_tot_time_sec : 0) + "% ОПМ)\n"
        "Средняя продолжительность непрерывной медитации: " + String(med_asd_time_sec) + " с. (" + String(med_tot_time_sec ? (med_asd_time_sec * 100) / med_tot_time_sec : 0) + "% ОПМ)\n"
        "Максимальное значение уровня медитации: " + String(max_med_val) + "\n"
        "Среднее значение уровня медитации: " + String(avg_med_val, 1);

    return res;
}
}