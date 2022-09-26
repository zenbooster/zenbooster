#pragma once
#include <Preferences.h>
#include <string>

namespace FormulaDB
{
using namespace std;

class TFormulaDB
{
    private:
        const string name;
        const string name_list;
        Preferences prefs;

    public:
        TFormulaDB(const string name);

        bool assign(const string key, const string val);
};
}