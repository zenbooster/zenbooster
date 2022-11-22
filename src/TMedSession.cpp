#include "TMedSession.h"
#include "TMyApplication.h"
#include "TWorker/TWorker.h"
#include "TConf.h"
#include "TUtil.h"
#include "TWiFiStuff.h"
#include "TConf.h"

namespace MedSession
{
using namespace Worker;
using namespace Util;
using namespace MyApplication;

uint16_t TMedSession::minsessec;
bool TMedSession::is_minsessec;
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
    TWorker::println("TMedSession::TMedSession()");
    is_minsessec = false;
    sess_beg = TWiFiStuff::time_cli.getEpochTime();
    // по хорошему, если порог или предпорог поменялись пока сеанс был открыт,
    // надо его закрыть, применить изменения порогов и затем снова открыть...
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    tr = TMyApplication::threshold;
    pretr = TMyApplication::pre_threshold;
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);
}

TMedSession::~TMedSession()
{
    //TWorker::println("TMedSession::~TMedSession()");
    Serial.println("TMedSession::~TMedSession()");
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    if(is_minsessec)
    {
        TWiFiStuff::tgb_send(
            "*Отчёт по сеансу:*\n`" + 
            TUtil::screen_mark_down(
            "Формула: " + formula_name + " = " + formula_text + "\n"
            "Порог: " + String(tr) + "\n"
            "Предпорог: " + String(pretr) + "\n" +
            gen_report()
            ) + 
            "`"
        );
        
        /*TWiFiStuff::tgb_send(
            TUtil::sprintf("*Отчёт по сеансу:*\n`%s'",
                TUtil::screen_mark_down(
                    TUtil::sprintf(
                        "Формула: %s = %s\n"
                        "Порог: %d\n"
                        "Предпорог: %d\n"
                        "%s",
                        formula_name.c_str(), formula_text.c_str(), tr, pretr, gen_report().c_str()
                    )
                ).get()
            )
        );*/
    }
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);
}

void TMedSession::calc_next(int32_t med)
{
    // данные приходят раз в секунду, по этому обойдёмся простым инкрементом:
    sess_time_sec++;

    if(!is_minsessec)
    {
        xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
        uint16_t mss = minsessec;
        xSemaphoreGiveRecursive(TConf::xOptRcMutex);
        is_minsessec = (sess_time_sec >= mss);

        if(is_minsessec)
        {
            TWorker::println("TMedSession::calc_next(..): По окончании сеанса будет сформирован отчёт.");
        }
    }

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
        "Начало сеанса: " + TWiFiStuff::time_cli.getFormattedDate(sess_beg) + "\n"
        "Продолжительность сеанса: " + String(sess_time_sec) + " с.\n"
        "Общая продолжительность медитации (ОПМ): " + String(med_tot_time_sec) + " с. (" + String((med_tot_time_sec * 100) / sess_time_sec) + "%)\n"
        "Общее количество непрерывных медитаций: " + String(med_sd_count) + "\n"
        "Максимальная продолжительность непрерывной медитации: " + String(med_msd_time_sec) + " с. (" + String(med_tot_time_sec ? (med_msd_time_sec * 100) / med_tot_time_sec : 0) + "% ОПМ)\n"
        "Средняя продолжительность непрерывной медитации: " + String(med_asd_time_sec) + " с. (" + String(med_tot_time_sec ? (med_asd_time_sec * 100) / med_tot_time_sec : 0) + "% ОПМ)\n"
        "Максимальное значение уровня медитации: " + String(max_med_val) + "\n"
        "Среднее значение уровня медитации: " + String(avg_med_val, 1);

    return res;
}
}