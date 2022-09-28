#include "TElementsDB.h"

namespace ElementsDB
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

inline uint16_t TElementsDB::pop16(uint16_t x)
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

inline uint8_t TElementsDB::get_first_zero_bit(uint8_t x)
{
    uint16_t t = x;
    t = ~t;
    t = t ^ (t - 1);

    return pop16(t) - 1;
}

TFirstZeroBitResult TElementsDB::get_first_zero_bit()
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
    } while(i < bitmap_size && n == 8);

    return TFirstZeroBitResult(res, bitmap_size);
}

String TElementsDB::get_chunk_name(uint8_t i)
{
    return "bmp_" + String(i, 0x10);
}

void TElementsDB::init_chunks(void)
{
    Serial.println("TElementsDB::init_chunks(..): проверяем существование всех чанков, т.к. в процессе первичной инициализации могло отключиться питание...");
    for(int i = 0; i < bitmap_size; i++)
    {
        String s_chunk_name = get_chunk_name(i);
        const char *chunk_name = s_chunk_name.c_str();

        if(!prefs.isKey(chunk_name))
        {
            Serial.printf("TElementsDB::init_chunks(..): первичная инициализация чанка %s\n", chunk_name);
            prefs.putUChar(chunk_name, 0);
        }
    }
}

void TElementsDB::integrity_check(void)
{
    // Under construction...
    //Serial.println("TElementsDB::integrity_check(..): проверяем целостность БД...");
}

TElementsDB::TElementsDB(const String name):
    name(name),
    name_list(name + "-list")
{
    prefs.begin(name_list.c_str(), false);
    init_chunks();
    integrity_check();
    prefs.end();
}

TElementsDB::~TElementsDB()
{
}

void TElementsDB::write_bit(uint8_t n, bool is)
{
    uint8_t chunk_num = n >> 3;
    Serial.printf("TElementsDB::write_bit(..): номер чанка: \"%d\".\n", chunk_num);

    uint8_t bit_num = n - (chunk_num << 3);
    Serial.printf("TElementsDB::write_bit(..): номер бита в чанке: \"%d\".\n", bit_num);

    String s_chunk_name = get_chunk_name(chunk_num);
    
    prefs.begin(name_list.c_str(), false);
    uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
    Serial.printf("TElementsDB::write_bit(..): прочитали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2));

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

    Serial.printf("TElementsDB::write_bit(..): записали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2));
}

bool TElementsDB::assign(const String key, const String val)
{
    bool b_res = false;

    do // fake loop
    {
        prefs.begin(name.c_str(), false);
        bool is_key = prefs.isKey(key.c_str());
        prefs.end();
        uint16_t id = 0;

        if(is_key)
        {
            Serial.printf("TElementsDB::assign(..): ключ \"%s\" существует.\n", key.c_str());
            prefs.begin(name.c_str(), false);
            String value = get_value_id(key, (uint8_t*)&id);
            prefs.end();

            if(value == val)
            {
                Serial.println("TElementsDB::assign(..): Новое значение совпадает со старым.");
                b_res = true;
                break;
            }
        }

        if(val.length())
        {
            Serial.printf("TElementsDB::assign(..): добавление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                Serial.printf("TElementsDB::assign(..): ключ \"%s\" ещё не существует.\n", key.c_str());
                prefs.begin(name_list.c_str(), false);
                // найти номер первого нулевого бита в битовой карте:
                TFirstZeroBitResult fzb = get_first_zero_bit();
                prefs.end();

                if(!fzb.check())
                {
                    Serial.println("TElementsDB::assign(..): в битовой карте не осталось места!");
                    break; // не осталось места
                }

                id = fzb;

                write_bit(id, true);

                Serial.printf("TElementsDB::assign(..): id = %d\n", id);
                Serial.println("TElementsDB::assign(..): записываем ссылку на элемент в список.");
                prefs.begin(name_list.c_str(), false);
                prefs.putString(String(id, 0x10).c_str(), key.c_str());
                prefs.end();
            }

            prefs.begin(name.c_str(), false);
            Serial.println("TElementsDB::assign(..): записываем элемент.");

            size_t sz_data = 1 + val.length();
            uint8_t *p_data = new uint8_t[sz_data];

            p_data[0] = id;
            memcpy(p_data + 1, val.c_str(), sz_data - 1);
            prefs.putBytes(key.c_str(), p_data, sz_data);
            delete [] p_data;
            prefs.end();
        }
        else
        {
            Serial.printf("TElementsDB::assign(..): удаление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                Serial.println("TElementsDB::assign(..): попытка удалить несуществующий ключ!");
                break;
            }

            // Сбросить соответствующий бит в битовой карте:
            write_bit(id, false);

            Serial.println("TElementsDB::assign(..): удаляем элемент.");
            prefs.begin(name.c_str(), false);
            prefs.remove(key.c_str());
            prefs.end();
        }
        b_res = true;
    } while (false);

    return b_res;
}

String screen_mark_down(const String s)
{
    String res = s;
    char c2r[] = "\\`~!@#$%^&*()-_=+[{]}|;:'\",<.>/?";

    char *p = c2r;
    char *p_end = p + sizeof(c2r);
    for(; p < p_end;)
    {
        char c = *p++;

        if(res.indexOf(c) > -1)
        {
            char src[] = " ";
            char dst[] = "\\ ";

            src[0] = c;
            dst[1] = c;
            res.replace(src, dst);
        }
    }

    return res;
}

String TElementsDB::list(void)
{
    String res;
    uint8_t i = 0;
    uint8_t j = 0;

    do
    {
        uint8_t t = 0;
        uint8_t n = 0;
        String s_chunk_name = get_chunk_name(i);
        prefs.begin(name_list.c_str(), false);
        uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
        prefs.end();

        bool is_skip_zeroes = !(chunk & 1);

        for(; n < 8; is_skip_zeroes = !is_skip_zeroes)
        {
            if(is_skip_zeroes)
            {
                chunk = ~chunk;
                n = get_first_zero_bit(chunk);

                if(n == 8)
                {
                    j += 8 - t;
                    break;
                }
                t += n;
                j += n;
    
                chunk >>= n;
                chunk = ~chunk;
                chunk &= 0b11111111 >> t;
            }
            else
            {
                n = get_first_zero_bit(chunk);

                for(uint8_t k = 0; k < n; k++)
                {
                    String key;
                    prefs.begin(name_list.c_str(), false);
                    key = prefs.getString(String(j++, 0x10).c_str());
                    prefs.end();

                    res += "*" + ElementsDB::screen_mark_down(key) + "* \\= `";

                    prefs.begin(name.c_str(), false);
                    res += ElementsDB::screen_mark_down(get_value_id(key));
                    prefs.end();

                    res += "`\n";
                }
                chunk >>= n;
                t += n;
            }
        } // for(; n < 8; is_skip_zeroes = !is_skip_zeroes)
    } while(++i < bitmap_size);

    return res;
}

String TElementsDB::get_value_id(const String key, uint8_t *id)
{
    String res;

    size_t sz_data = prefs.getBytesLength(key.c_str());
    uint8_t *p_data = new uint8_t[sz_data];
    prefs.getBytes(key.c_str(), p_data, sz_data);

    if(id)
    {
        *id = *(uint8_t*)p_data;
    }

    char *t = new char[sz_data];
    memcpy(t, p_data + 1, sz_data - 1);
    t[sz_data - 1] = 0;
    res = t;

    delete [] t;
    delete [] p_data;

    return res;
}

String TElementsDB::get_value(const String key)
{
    String res;

    prefs.begin(name.c_str(), false);
    bool is_key = prefs.isKey(key.c_str());

    if(is_key)
    {
        res = get_value_id(key);
    }
    prefs.end();

    return res;
}
}