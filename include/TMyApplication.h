#pragma once
#include "common.h"
#include "WiFiManager.h"
#ifdef PIN_BTN
# include "TSleepMode.h"
#endif
#include "TTgamParsedValues.h"
#include "TRingBufferInItem.h"

namespace Conf {class TConf;}
namespace Noise {class TNoise;}
namespace BluetoothStuff {class TBluetoothStuff;}
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
    
    TConf *p_conf;
#ifdef PIN_BTN
    TSleepMode SleepMode;
#endif
    WiFiManager wifiManager;

    static int MED_THRESHOLD;// = 95;
    static int MED_PRE_TRESHOLD_DELTA;// = 10;

    TRingBufferInItem ring_buffer_in[4];
    int ring_buffer_in_index;
    int ring_buffer_in_size;

    // размер подобрать под ширину отображаемого графика:
    //TRingBufferOutItem ring_buffer_out[1024];
    //TRingBufferOutItem ring_buffer_out[512];
    //int ring_buffer_out_index;
    
  #ifdef SOUND
    TNoise *p_noise;
  #endif
    TBluetoothStuff *p_bluetooth_stuff;
    TWiFiStuff *p_wifi_stuff;
    static TCalcFormula *p_calc_formula;
    static SemaphoreHandle_t xCFSemaphore;
    
    static bool is_blink_on_packets; // мигнуть при поступлении нового пакета от гарнитуры?
    static bool is_blue_pulse;
    static bool is_log_data_to_bot;

    int calc_formula_meditation();
    static int int_from_12bit(const uint8_t *buf);
    static void callback(const TTgamParsedValues tpv, void *arg);
    static void update_calc_formula(TCalcFormula *pcf);

    friend class Conf::TConf;

  public:
    TMyApplication();
    ~TMyApplication();

    static String get_version_string(void);
    void run(void);
};
}