#include "TFormulaDB.h"

namespace FormulaDB
{
class TFirstZeroBitResult
{
    private:
        uint16_t res;
        size_t len;

    public:
        TFirstZeroBitResult(uint16_t res, size_t len);

        bool check(void) const;
        operator uint8_t() const;
};

TFirstZeroBitResult::TFirstZeroBitResult(uint16_t res, size_t len):
    res(res),
    len(len)
{
}

bool TFirstZeroBitResult::check(void) const
{
    return res < (len << 3);
}

TFirstZeroBitResult::operator uint8_t() const
{
    return res;
}

inline uint16_t TFormulaDB::pop16(uint16_t x)
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

inline uint8_t TFormulaDB::get_first_zero_bit(uint8_t x)
{
    uint16_t t = x;
    t = ~t;
    t = t ^ (t - 1);

    return pop16(t) - 1;
}

TFirstZeroBitResult TFormulaDB::get_first_zero_bit(size_t len)
{
    uint16_t res = 0;
    uint8_t i = 0;
    uint8_t n;

    do
    {
        String s_chunk_name = get_chunk_name(i++);
        uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
        n = get_first_zero_bit(chunk);

        res += n;

        if(n == 8)
        {
            continue;
        }
    } while(i < len && n == 8);

    return TFirstZeroBitResult(res, len);
}

String TFormulaDB::get_chunk_name(uint8_t i)
{
    return "bmp_" + String(i, 0x10);
}

TFormulaDB::TFormulaDB(const string name):
    name(name),
    name_list(name + "-list")
{
    prefs.begin(name_list.c_str(), false);

    for(int i = 0; i < 32; i++)
    {
        String s_chunk_name = get_chunk_name(i);
        const char *chunk_name = s_chunk_name.c_str();

        // проверяем существование всех чанков, т.к. в процессе первичной инициализации могло отключиться питание:
        if(!prefs.isKey(chunk_name))
        {
            Serial.printf("TFirstZeroBitResult::TFirstZeroBitResult(..): первичная инициализация чанка %s\n", chunk_name);
            prefs.putUChar(chunk_name, 0);
        }
    }

    prefs.end();
}

TFormulaDB::~TFormulaDB()
{
}

void TFormulaDB::write_bit(uint8_t n, bool is)
{
    uint8_t chunk_num = n >> 3;
    Serial.printf("TFormulaDB::write_bit(..): номер чанка: \"%d\".\n", chunk_num);

    uint8_t bit_num = n - (chunk_num << 3);
    Serial.printf("TFormulaDB::write_bit(..): номер бита в чанке: \"%d\".\n", bit_num);

    String s_chunk_name = get_chunk_name(chunk_num);
    
    prefs.begin(name_list.c_str(), false);
    uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
    Serial.printf("TFormulaDB::write_bit(..): прочитали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2));

    if(is)
    {
        chunk |= 1 << bit_num; // устанавливаем бит
    }
    else
    {
        chunk &= ~(1 << bit_num); // сбрасываем бит
    }

    prefs.putUChar(s_chunk_name.c_str(), chunk);
    prefs.end();

    Serial.printf("TFormulaDB::write_bit(..): записали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2));
}

bool TFormulaDB::assign(const string key, const string val)
{
    bool b_res = false;

    do // fake loop
    {
        prefs.begin(name.c_str(), false);
        bool is_key = prefs.isKey(key.c_str());
        uint16_t id;

        if(is_key)
        {
            Serial.printf("TFormulaDB::assign(..): ключ \"%s\" существует.\n", key.c_str());
            size_t sz_data = prefs.getBytesLength(key.c_str());
            uint8_t *p_data = new uint8_t[sz_data];
            prefs.getBytes(key.c_str(), p_data, sz_data);

            string value((char*)p_data + 1, sz_data - 1);

            if(value == val)
            {
                Serial.println("TFormulaDB::assign(..): Новое значение совпадает со старым.");
                b_res = true;
                break;
            }

            id = p_data[0];
        }
        prefs.end();

        if(val.empty())
        {
            Serial.printf("TFormulaDB::assign(..): удаление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                Serial.println("TFormulaDB::assign(..): попытка удалить несуществующий ключ!");
                break;
            }

            // Сбросить соответствующий бит в битовой карте:
            write_bit(id, false);

            Serial.println("TFormulaDB::assign(..): удаляем формулу.");
            prefs.begin(name.c_str(), false);
            prefs.remove(key.c_str());
            prefs.end();
        }
        else
        {
            Serial.printf("TFormulaDB::assign(..): добавление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                Serial.printf("TFormulaDB::assign(..): ключ \"%s\" ещё не существует.\n", key.c_str());
                prefs.begin(name_list.c_str(), false);
                // найти номер первого нулевого бита в битовой карте:
                TFirstZeroBitResult fzb = get_first_zero_bit(bitmap_size);
                if(!fzb.check())
                {
                    Serial.println("TFormulaDB::assign(..): в битовой карте не осталось места!");
                    break; // не осталось места
                }

                id = fzb;

                write_bit(id, true);

                Serial.printf("TFormulaDB::assign(..): id = %d\n", id);
                Serial.println("TFormulaDB::assign(..): записываем ссылку на формулу в список.");
                prefs.putString(String(id, 0x10).c_str(), key.c_str());
                prefs.end();
            }

            prefs.begin(name.c_str(), false);
            Serial.println("TFormulaDB::assign(..): записываем формулу.");

            size_t sz_data = 1 + val.length();
            uint8_t *p_data = new uint8_t[sz_data];

            p_data[0] = id;
            memcpy(p_data + 1, val.c_str(), sz_data - 1);
            prefs.putBytes(key.c_str(), p_data, sz_data);

            prefs.end();
        }
        b_res = true;
    } while (false);

    return b_res;
}
}