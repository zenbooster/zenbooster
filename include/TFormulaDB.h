#pragma once
#include <Preferences.h>
#include <string>

namespace FormulaDB
{
using namespace std;

class TFirstZeroBitResult;

class TFormulaDB
{
    private:
        const size_t bitmap_size = 32;
        const string name;
        const string name_list;
        Preferences prefs;

        static inline String get_chunk_name(uint8_t i);

        static inline uint16_t pop16(uint16_t x);
        static inline uint8_t get_first_zero_bit(uint8_t x);
        TFirstZeroBitResult get_first_zero_bit();

        void write_bit(uint8_t n, bool is);

    public:
        TFormulaDB(const string name);
        ~TFormulaDB();

        bool assign(const string key, const string val);
        String list(void);
};
}