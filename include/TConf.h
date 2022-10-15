#pragma once
#include "TPrefs.h"
#include "TFormulaDB.h"

namespace Conf
{
using namespace Prefs;
using namespace FormulaDB;

class TConf
{
    private:
        TPrefs *p_prefs;
        TFormulaDB *p_fdb;

        const char *key_options = "options";
        const char *key_formulas = "formulas";

    public:
        TConf();
        ~TConf();

        TPrefs *get_prefs();
        TFormulaDB *get_fdb();

        DynamicJsonDocument get_json(void);
        void validate_json(const DynamicJsonDocument& doc);
        void add_json(const DynamicJsonDocument& doc);
        void set_json(const DynamicJsonDocument& doc);
};
}