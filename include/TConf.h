#pragma once
#include "TPrefs.h"
#include "TFormulaDB.h"

namespace MyApplication { class TMyApplication; }

namespace Conf
{
using namespace Prefs;
using namespace FormulaDB;
using namespace MyApplication;

class TConf
{
    private:
        TPrefs *p_prefs;
        TFormulaDB *p_fdb;

    public:
        TConf(TMyApplication *p_app);
        ~TConf();

        TPrefs *get_prefs();
        TFormulaDB *get_fdb();

        DynamicJsonDocument get_json(void);
        void validate_json(DynamicJsonDocument& doc);
};
}