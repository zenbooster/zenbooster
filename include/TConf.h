#pragma once
#include "TPrefs.h"
#include "TFormulaDB.h"

namespace MyApplication {class TMyApplication;}
namespace BluetoothStuff {class TBluetoothStuff;}
namespace WiFiStuff {class TWiFiStuff;}
namespace ButtonIllumination {class TButtonIllumination;}
namespace MedSession {class TMedSession;}

namespace Conf
{
using namespace Prefs;
using namespace FormulaDB;
using namespace MyApplication;
using namespace BluetoothStuff;
using namespace WiFiStuff;
using namespace ButtonIllumination;
using namespace MedSession;

class TConf
{
    private:
        static SemaphoreHandle_t xOptRcMutex;
        static TPrefs *p_prefs;
        static TFormulaDB *p_fdb;

        static const char *key_options;
        static const char *key_formulas;

        friend class MyApplication::TMyApplication;
        friend class BluetoothStuff::TBluetoothStuff;
        friend class WiFiStuff::TWiFiStuff;
        friend class ButtonIllumination::TButtonIllumination;
        friend class MedSession::TMedSession;

    public:
        TConf();
        ~TConf();

        static TPrefs *get_prefs();
        static TFormulaDB *get_fdb();

        static DynamicJsonDocument get_json(void);
        static void validate_json(const DynamicJsonDocument& doc);
        static void add_json(const DynamicJsonDocument& doc);
        static void set_json(const DynamicJsonDocument& doc);
};
}