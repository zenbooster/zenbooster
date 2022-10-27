#include "TButtonIllumination.h"
#include "TConf.h"
#include "common.h"
#include "TUtil.h"
#include <esp32-hal-ledc.h>

namespace ButtonIllumination
{
using namespace MyApplication;
using namespace Util;

bool TButtonIllumination::is_blink_on_packets = false;
bool TButtonIllumination::is_led_pulse = true;
uint8_t TButtonIllumination::led_pulse_id;
bool TButtonIllumination::is_poor_signal_indicated = false;
float TButtonIllumination::max_illumination_level;

TButtonIllumination::TButtonIllumination()
{
    ledcSetup(0, 40, 8);
    ledcSetup(1, 40, 8);
    ledcSetup(2, 40, 8);

    ledcAttachPin(PIN_LED_R, 0);
    ledcAttachPin(PIN_LED_G, 1);
    ledcAttachPin(PIN_LED_B, 2);

    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t vmax = TUtil::percent_of(max_illumination_level, 255);
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    ledcWrite(0, vmax);
}

TButtonIllumination::~TButtonIllumination()
{
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t vmax = TUtil::percent_of(max_illumination_level, 255);
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    ledcWrite(0, vmax);
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWriteTone(0, 12);
}

void TButtonIllumination::on_wait_for_connect(void)
{
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t v = TUtil::percent_of(max_illumination_level, 255);
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    ledcWrite(0, v >> 2);
    ledcWrite(1, 0);
    ledcWrite(2, 0);
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
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t vmax = TUtil::percent_of(max_illumination_level, 255);
    bool is_bod = is_blink_on_packets;
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    if(is_bod)
    {
        ledcWrite(led_pulse_id, is_led_pulse ? vmax: (vmax >> 1));
        is_led_pulse = !is_led_pulse;
    }
    else
    {
        ledcWrite(led_pulse_id, vmax);
    }
}

void TButtonIllumination::on_threshold_reached(void)
{
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t vmax = TUtil::percent_of(max_illumination_level, 255);
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    ledcWrite(0, vmax);
    ledcWrite(1, vmax);
}

void TButtonIllumination::on_pre_threshold_reached(int d, int threshold)
{
    xSemaphoreTakeRecursive(TConf::xOptRcMutex, portMAX_DELAY);
    uint8_t vmax = TUtil::percent_of(max_illumination_level, 255);
    xSemaphoreGiveRecursive(TConf::xOptRcMutex);

    int led_lvl = vmax - (d * vmax) / threshold;
    ledcWrite(0, led_lvl);
    ledcWrite(1, led_lvl);
}

void TButtonIllumination::on_pre_threshold_not_reached(void)
{
    ledcWrite(0, 0);
    ledcWrite(1, 0);
}
}