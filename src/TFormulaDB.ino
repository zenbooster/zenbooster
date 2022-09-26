#include "TFormulaDB.h"

namespace FormulaDB
{
inline uint16_t pop16(uint16_t x)
{
	const uint16_t m1 = 0x5555; //binary: 0101...
    const uint16_t m2 = 0x3333; //binary: 00110011..
    const uint16_t m4 = 0x0f0f; //binary:  4 zeros,  4 ones ...

    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
    x += x >> 8;
    return x & 0x7f;
}

inline uint8_t get_first_zero_bit(uint8_t x)
{
    uint16_t t = x;
    t = ~t;
    t = t ^ (t - 1);

    return pop16(t) - 1;
}

TFormulaDB::TFormulaDB(const string name):
    name(name),
    name_list(name + "-list")
{
    prefs.begin(name_list.c_str(), false);
    if(!prefs.isKey("bitmap"))
    {
        uint8_t bitmap[32] = {};
        prefs.putBytes("bitmap", bitmap, 256);
    }
    prefs.end();
}

bool TFormulaDB::assign(const string key, const string val)
{
    bool b_res = false;

    do // fake loop
    {
        prefs.begin(name.c_str(), false);
        bool is_key = prefs.isKey(key.c_str());

        if(is_key)
        {
            string value = prefs.getString(key.c_str()).c_str();
            if(value == val)
            {
                b_res = true;
                break;
            }
        }
        prefs.end();

        if(!is_key) // новый элемент
        {
            prefs.begin(name_list.c_str(), false);
            uint8_t bitmap[32];

            prefs.getBytes("bitmap", bitmap, 256);

            uint8_t id;
            // найти номер первого нулевого бита в битовой карте:
            //
            prefs.putString(String(id).c_str(), val.c_str());
            prefs.end();
        }

        prefs.begin(name.c_str(), false);
        prefs.putString(key.c_str(), val.c_str());
        prefs.end();
        b_res = true;
    } while (false);

    return b_res;
}
}