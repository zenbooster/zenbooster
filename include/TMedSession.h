#pragma once
#include <stdint.h>
#include <WString.h>
#include <functional>

namespace MedSession
{
using namespace std;

class TMedSession
{
    private:
        uint32_t threshold;

        uint16_t sess_time_sec; // продолжительность сессии в секундах
        uint16_t med_tot_time_sec; // общая (суммарная) продолжительность медитации
        uint16_t med_sd_time_sec; // solid time - текущая продолжительность непрерывной медитации
        uint16_t med_msd_time_sec; // max solid time - максимальная продолжительность непрерывной медитации
        double avg_med_val; // среднее значение уровня медитации

    public:
        TMedSession(uint32_t threshold);

        void calc_next(int32_t med);
        String gen_report(void) const;
};
}