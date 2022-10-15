#pragma once
#include <stdint.h>
#include <WString.h>
#include <functional>

namespace WiFiStuff {class TWiFiStuff;}
namespace Conf {class TConf;}

namespace MedSession
{
using namespace std;
using namespace WiFiStuff;
using namespace Conf;

class TMedSession
{
    private:
        TWiFiStuff *p_wifi_stuff;
        static uint32_t threshold;
        static uint32_t pre_threshold;
        static uint16_t minsessec;
        static String formula_name;
        static String formula_text;

        uint16_t sess_time_sec; // продолжительность сессии в секундах
        uint16_t med_tot_time_sec; // общая (суммарная) продолжительность медитации
        uint16_t med_sd_time_sec; // solid time - текущая продолжительность непрерывной медитации
        uint16_t med_msd_time_sec; // max solid time - максимальная продолжительность непрерывной медитации
        uint32_t max_med_val; // максимальное значение уровня медитации
        double avg_med_val; // среднее значение уровня медитации

        friend class Conf::TConf;

    public:
        TMedSession(TWiFiStuff *p_wifi_stuff);
        ~TMedSession();

        void calc_next(int32_t med);
        String gen_report(void) const;
};
}