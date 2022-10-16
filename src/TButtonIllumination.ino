#include "TButtonIllumination.h"
#include "common.h"
#include <esp32-hal-ledc.h>

namespace ButtonIllumination
{
bool TButtonIllumination::is_blink_on_packets = false;
bool TButtonIllumination::is_led_pulse = true;
uint8_t TButtonIllumination::led_pulse_id;
bool TButtonIllumination::is_poor_signal_indicated = false;

TButtonIllumination::TButtonIllumination()
{
    //
}

void TButtonIllumination::on_msession_disconnect(void)
{
    is_poor_signal_indicated = false;
}

void TButtonIllumination::on_msession_poor_signal(void)
{
    if(!is_poor_signal_indicated)
    {
        ledcWrite(0, 0);
        ledcWrite(2, 0);
        led_pulse_id = 1;
        is_poor_signal_indicated = true;
    }
}

void TButtonIllumination::on_msession_connect(void)
{
    ledcWrite(1, 0);
    led_pulse_id = 2;
    is_poor_signal_indicated = false;
}

void TButtonIllumination::on_msession_data(void)
{
    if(is_blink_on_packets)
    {
        ledcWrite(led_pulse_id, is_led_pulse ? 255: 128);
        is_led_pulse = !is_led_pulse;
    }
    else
    {
        ledcWrite(led_pulse_id, 255);
    }
}

void TButtonIllumination::on_threshold_reached(void)
{
    ledcWrite(0, 255);
    ledcWrite(1, 255);
}

void TButtonIllumination::on_pre_threshold_reached(int d, int threshold)
{
    int led_lvl = 255 - (d * 255) / threshold;
    ledcWrite(0, led_lvl);
    ledcWrite(1, led_lvl);
}

void TButtonIllumination::on_pre_threshold_not_reached(void)
{
    ledcWrite(0, 0);
    ledcWrite(1, 0);
}
}