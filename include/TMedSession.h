#pragma once
#include <stdint.h>
#include <WString.h>
#include <functional>
#include <ArduinoJson.h>
#include "TTgamParsedValues.h"

namespace WiFiStuff {class TWiFiStuff;}
namespace Conf {class TConf;}

namespace MedSession
{
using namespace std;
using namespace WiFiStuff;
using namespace Conf;
using namespace TgamParsedValues;

class TMedSession
{
    private:
        unsigned long sess_beg;
        int tr;
        int pretr;
        static uint16_t minsessec;
        static bool is_minsessec;
        static String formula_name;
        static String formula_text;

        uint16_t sess_time_sec; // продолжительность сеанса в секундах
        uint16_t med_tot_time_sec; // общая (суммарная) продолжительность медитации
        uint16_t med_sd_time_sec; // solid time - текущая продолжительность непрерывной медитации
        uint16_t med_sd_count; // общее количество непрерывных медитаций
        uint16_t med_msd_time_sec; // max solid time - максимальная продолжительность непрерывной медитации
        uint16_t med_asd_time_sec; // average solid time - средняя продолжительность непрерывной медитации
        uint32_t max_med_val; // максимальное значение уровня медитации
        double avg_med_val; // среднее значение уровня медитации

        String gen_report() const;

        friend class Conf::TConf;

    public:
        TMedSession();
        ~TMedSession();

        void calc_next(TTgamParsedValues *p, int32_t med);
};
}