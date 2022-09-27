#pragma once
#include <Preferences.h>
#include <string>

namespace FormulaDB
{
using namespace std;

class TFirstZeroBit;

class TFormulaDB
{
    private:
        const string name;
        const string name_list;
        Preferences prefs;
        TFirstZeroBit *p_fzb;

        void write_bit(uint8_t n, bool is);

    public:
        TFormulaDB(const string name);
        ~TFormulaDB();

        bool assign(const string key, const string val);
};
}