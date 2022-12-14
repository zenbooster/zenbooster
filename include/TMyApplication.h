#pragma once
#include "common.h"
#include "TSingleton.h"
#include "WiFiManager.h"
#ifdef PIN_BTN
# include "TSleepMode.h"
#endif
#include "TTgamParsedValues.h"
#include "TRingBufferInItem.h"
#include "TBluetoothStuff.h"
#include "TMedSession.h"
#include "TButtonIllumination.h"

namespace Conf {class TConf;}
namespace Worker {class TWorker;}
namespace MedSession {class TMedSession;}
namespace Noise {class TNoise;}
namespace WiFiStuff {class TWiFiStuff;}
namespace CalcFormula {class TCalcFormula;}

namespace MyApplication
{
using namespace Conf;
using namespace MedSession;
#ifdef SOUND
using namespace Noise;
#endif
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace CalcFormula;

using namespace Worker;
#ifdef PIN_BTN
using namespace SleepMode;
#endif
using namespace TgamParsedValues;
using namespace RingBufferInItem;
using namespace MedSession;
using namespace ButtonIllumination;

/*struct TRingBufferOutItem
{
  time_t time;
  int meditation;
};*/

class TMyApplication: public TSingleton<TMyApplication>
{
  private:
    static const char *DEVICE_NAME_FULL;
    static const char *DEVICE_NAME;
    static const char *WIFI_SSID;
    static const char *WIFI_PASS;
    
    static TConf *p_conf;
    static TWorker *p_worker;
#ifdef PIN_BTN
    static TSleepMode *p_sleep_mode;
#endif
    static WiFiManager *p_wifi_manager;
    static WiFiManagerParameter tgm_bot_token;
    static WiFiManagerParameter tgm_chat_id;

    static int threshold;
    static int pre_threshold;

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
    
    static int i_log_data_to_bot;

    static bool is_use_poor_signal;
#ifdef PIN_BTN
    static TButtonIllumination *p_btn_il;
#endif
    static TMedSession *p_med_session;
    static String s_log_data;
    static int i_log_data;

    static int calc_formula_meditation();
    static int int_from_12bit(const uint8_t *buf);
    static void callback(const TTgamParsedValues *p_tpv, TCallbackEvent evt);
    static void update_calc_formula(TCalcFormula *pcf);

    friend class Conf::TConf;
    friend class MedSession::TMedSession;

  public:
    TMyApplication();
    ~TMyApplication();

    static const String get_version_string(void);
    void run(void);
};
}