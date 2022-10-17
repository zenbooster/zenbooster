#pragma once
#include <stdint.h>

namespace Conf {class TConf;}

namespace ButtonIllumination
{
using namespace Conf;

class TButtonIllumination
{
    private:
        static bool is_blink_on_packets; // мигнуть при поступлении нового пакета от гарнитуры?
        static bool is_led_pulse;
        static uint8_t led_pulse_id;
        //static bool is_poor_signal_indicated;
        static float max_illumination_level;

        friend class Conf::TConf;
    
    public:
        TButtonIllumination();

        static bool is_poor_signal_indicated;

        static void on_wait_for_connect(void);

        static void on_msession_connect(void);
        static void on_msession_disconnect(void);
        static void on_msession_poor_signal(void);
        static void on_msession_data(void);

        static void on_threshold_reached(void);
        static void on_pre_threshold_reached(int d, int threshold);
        static void on_pre_threshold_not_reached(void);
};
}