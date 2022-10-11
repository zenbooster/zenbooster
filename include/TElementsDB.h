#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

namespace ElementsDB
{
using namespace std;

typedef function<void(const uint8_t id)> TCbTraverseFunction;

class TFirstZeroBitResult;

class TElementsDB
{
    private:
        const size_t bitmap_size = 32; // 32 * 8 = 256 bit
        const String name;
        const String name_list;
        Preferences prefs;

        static inline String get_chunk_name(uint8_t i);

        static inline uint16_t pop16(uint16_t x);
        static inline uint8_t get_first_zero_bit(uint8_t x);
        TFirstZeroBitResult get_first_zero_bit();

        void write_bit(uint8_t n, bool is);
        String get_value_id(const String& key, uint8_t *id = NULL);

        void init_chunks(void);
        bool integrity_check(void);

        void chk_key(const String& key); // может бросить исключение
        String get_key_by_id(const uint8_t id);

        void traverse(TCbTraverseFunction cb); // обойти все элементы

    public:
        TElementsDB(const String& name);
        ~TElementsDB();

        bool has_value(const String& key); // может бросить исключение
        void assign(const String& key, const String& val = ""); // может бросить исключение
        String list(const String *p_current_key = NULL);
        DynamicJsonDocument get_json(void);
        bool is_empty(void);
        String get_value(const String& key); // может бросить исключение
};
}