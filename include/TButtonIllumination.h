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
        static bool is_use_poor_signal;
        //static bool is_poor_signal_indicated;

        friend class Conf::TConf;
    
    public:
        TButtonIllumination();

        static bool is_poor_signal_indicated;

        void on_msession_connect(void);
        void on_msession_disconnect(void);
        void on_msession_poor_signal(void);
        void on_msession_data(void);
};
}