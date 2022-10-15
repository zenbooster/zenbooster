#pragma once
#include "common.h"
#include "WiFiManager.h"
#ifdef PIN_BTN
# include "TSleepMode.h"
#endif
#include "TTgamParsedValues.h"
#include "TRingBufferInItem.h"
#include "TBluetoothStuff.h"
#include "TMedSession.h"

namespace Conf {class TConf;}
namespace Noise {class TNoise;}
namespace WiFiStuff {class TWiFiStuff;}
namespace CalcFormula {class TCalcFormula;}

namespace MyApplication
{
using namespace Conf;
#ifdef SOUND
using namespace Noise;
#endif
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace CalcFormula;

#ifdef PIN_BTN
using namespace SleepMode;
#endif
using namespace TgamParsedValues;
using namespace RingBufferInItem;
using namespace MedSession;

/*struct TRingBufferOutItem
{
  time_t time;
  int meditation;
};*/

class TMyApplication
{
  private:
    static const char *DEVICE_NAME_FULL;
    static const char *DEVICE_NAME;
    static const char *WIFI_SSID;
    static const char *WIFI_PASS;
    
    static TConf *p_conf;
#ifdef PIN_BTN
    TSleepMode SleepMode;
#endif
    WiFiManager wifiManager;

    static int MED_THRESHOLD;// = 95;
    static int MED_PRE_TRESHOLD_DELTA;// = 10;

    static TRingBufferInItem ring_buffer_in[4];
    static int ring_buffer_in_index;
    static int ring_buffer_in_size;

    // размер подобрать под ширину отображаемого графика:
    //TRingBufferOutItem ring_buffer_out[1024];
    //TRingBufferOutItem ring_buffer_out[512];
    //int ring_buffer_out_index;
    
  #ifdef SOUND
    static TNoise *p_noise;
  #endif
    static TBluetoothStuff *p_bluetooth_stuff;
    static TWiFiStuff *p_wifi_stuff;
    static TCalcFormula *p_calc_formula;
    static SemaphoreHandle_t xCFSemaphore;
    
    static bool is_blink_on_packets; // мигнуть при поступлении нового пакета от гарнитуры?
    static bool is_led_pulse;
    static uint8_t led_pulse_id;
    static bool is_log_data_to_bot;
    static bool is_use_poor_signal;
    static bool is_poor_signal_indicated;

    static TMedSession *p_med_session;

    static int calc_formula_meditation();
    static int int_from_12bit(const uint8_t *buf);
    static void callback(const TTgamParsedValues *p_tpv, TCallbackEvent evt);
    static void update_calc_formula(TCalcFormula *pcf);

    friend class Conf::TConf;

  public:
    TMyApplication();
    ~TMyApplication();

    static String get_version_string(void);
    void run(void);
};
}