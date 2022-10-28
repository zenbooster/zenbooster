#include "TElementsDB.h"
#include "TUtil.h"
#include "TWorker/TWorker.h"

namespace ElementsDB
{
using namespace Util;
using namespace Worker;

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
    } while(i < bitmap_size && n == 8);

    return TFirstZeroBitResult(res, bitmap_size);
}

String TElementsDB::get_chunk_name(uint8_t i)
{
    return "bmp_" + String(i, 0x10);
}

void TElementsDB::init_chunks(void)
{
    TWorker::print("TElementsDB::init_chunks(..): проверяем существование всех чанков, т.к. в процессе первичной инициализации могло отключиться питание...");
    prefs.begin(name_list.c_str(), false);
    for(int i = 0; i < bitmap_size; i++)
    {
        String s_chunk_name = get_chunk_name(i);
        const char *chunk_name = s_chunk_name.c_str();

        if(!prefs.isKey(chunk_name))
        {
            TWorker::printf("TElementsDB::init_chunks(..): первичная инициализация чанка %s\n", chunk_name);
            prefs.putUChar(chunk_name, 0);
        }
    }
    prefs.end();
    TWorker::println("Ok!");
}

bool TElementsDB::integrity_check(void)
{
    bool res = true;
    // Under construction...
    TWorker::print("TElementsDB::integrity_check(..): проверяем целостность БД...");
    traverse(
        [this, &res](const uint8_t id) -> void
        {
            String key = get_key_by_id(id);
            if(key.isEmpty())
            {
                TWorker::printf(
                    "\nTElementsDB::integrity_check(..): name_list не содержит ключа для id=%" PRIu8 "!"
                    " Сбрасываем соответствующий бит в битовой карте.\n", id
                );
                write_bit(id, false);
                res = false;
            }
            else
            {
                if(get_value_id(key).isEmpty())
                {
                    TWorker::printf(
                        "\nTElementsDB::integrity_check(..): name не содержит ключа \"%s\" для id=%" PRIu8 "!\n"
                        " Сбрасываем соответствующий бит в битовой карте.\n", key.c_str(), id
                    );
                    // При этом не трогаем пару id: key в name_list, т.к. в будущем её можно будет повторно использовать.
                    // Заодно сбережём ресурс флешки.
                    write_bit(id, false);
                    res = false;
                }
            }
        }
    );
    if(res)
    {
        TWorker::println("Ok!");
    }
    return res;
}

String TElementsDB::get_key_by_id(const uint8_t id)
{
    prefs.begin(name_list.c_str(), false);
    String key = prefs.getString(String(id, 0x10).c_str());
    prefs.end();
    return key;
}

TElementsDB::TElementsDB(const String& name):
    name(name),
    name_list(name + "-list")
{
    init_chunks();
    integrity_check();
}

TElementsDB::~TElementsDB()
{
}

void TElementsDB::write_bit(uint8_t n, bool is)
{
    uint8_t chunk_num = n >> 3;
    TWorker::printf("TElementsDB::write_bit(..): номер чанка: %" PRIu8 ".\n", chunk_num);

    uint8_t bit_num = n - (chunk_num << 3);
    TWorker::printf("TElementsDB::write_bit(..): номер бита в чанке: %" PRIu8 ".\n", bit_num);

    String s_chunk_name = get_chunk_name(chunk_num);
    
    prefs.begin(name_list.c_str(), false);
    uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
    TWorker::printf("TElementsDB::write_bit(..): прочитали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2).c_str());

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

    TWorker::printf("TElementsDB::write_bit(..): записали чанк \"%s\": 0b%s.\n", s_chunk_name.c_str(), String(chunk, 2).c_str());
}

bool TElementsDB::has_value(const String& key)
{
    TUtil::chk_nvs_key(key);
    prefs.begin(name.c_str(), false);
    bool is_key = prefs.isKey(key.c_str());
    prefs.end();

    return is_key;
}

void TElementsDB::assign(const String& key, const String& val)
{
    TUtil::chk_nvs_key(key);
    do // fake loop
    {
        prefs.begin(name.c_str(), false);
        bool is_key = prefs.isKey(key.c_str());
        prefs.end();
        uint16_t id = 0;

        if(is_key)
        {
            TWorker::printf("TElementsDB::assign(..): ключ \"%s\" существует.\n", key.c_str());
            String value = get_value_id(key, (uint8_t*)&id);

            if(value == val)
            {
                TWorker::println("TElementsDB::assign(..): Новое значение совпадает со старым.");
                break;
            }
        }

        if(val.length())
        {
            TWorker::printf("TElementsDB::assign(..): добавление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                TWorker::printf("TElementsDB::assign(..): ключ \"%s\" ещё не существует.\n", key.c_str());
                prefs.begin(name_list.c_str(), false);
                // найти номер первого нулевого бита в битовой карте:
                TFirstZeroBitResult fzb = get_first_zero_bit();
                prefs.end();

                if(!fzb.check())
                {
                    throw String("TElementsDB::assign(..): в битовой карте не осталось места");
                }

                id = fzb;

                write_bit(id, true);

                TWorker::printf("TElementsDB::assign(..): id = %" PRIu16 "\n", id);
                TWorker::println("TElementsDB::assign(..): записываем ссылку на элемент в список.");
                prefs.begin(name_list.c_str(), false);
                prefs.putString(String(id, 0x10).c_str(), key.c_str());
                prefs.end();
            }

            prefs.begin(name.c_str(), false);
            TWorker::println("TElementsDB::assign(..): записываем элемент.");

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
            TWorker::printf("TElementsDB::assign(..): удаление ключа \"%s\".\n", key.c_str());
            if(!is_key) // новый элемент
            {
                throw String("TElementsDB::assign(..): попытка удалить несуществующий ключ");
            }

            // Сбросить соответствующий бит в битовой карте:
            write_bit(id, false);

            TWorker::println("TElementsDB::assign(..): удаляем элемент.");
            prefs.begin(name.c_str(), false);
            prefs.remove(key.c_str());
            prefs.end();
        }
    } while (false);
}

void TElementsDB::traverse(TCbTraverseFunction cb)
{
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
                    cb(j++);
                }
                chunk >>= n;
                t += n;
            }
        } // for(; n < 8; is_skip_zeroes = !is_skip_zeroes)
    } while(++i < bitmap_size);
}

String TElementsDB::list(const String *p_current_key)
{
    String res;
    traverse(
        [this, &res, p_current_key](const uint8_t id) -> void
        {
            String key = get_key_by_id(id);

            String s = "`" + String((p_current_key && (key == *p_current_key)) ? "\\-\\>" : "  ") + "`*" + TUtil::screen_mark_down(key) + "* \\= `";

            s += TUtil::screen_mark_down(get_value_id(key));
            s += "`";

            res += s;
            res += "\n";
        }
    );
    return res;
}

void TElementsDB::clear(void)
{
    // удаляем всё:
    traverse(
        [this](const uint8_t id) -> void
        {
            String key = get_key_by_id(id);
            assign(key); // удаляем элемент
        }
    );
}

DynamicJsonDocument TElementsDB::get_json(void)
{
    DynamicJsonDocument res(1024);
    traverse(
        [this, &res](const uint8_t id) -> void
        {
            String key = get_key_by_id(id);

            res[key] = get_value_id(key);
        }
    );
    return res;
}

void TElementsDB::validate_json_iteration(JsonPairConst& kv)
{
    String key = kv.key().c_str();
    TUtil::chk_nvs_key(key);

    String val = kv.value().as<const char *>();
    if(val.isEmpty())
    {
        throw "значение для \"" + key + "\" не должно быть пустым";
    }
}

void TElementsDB::validate_json(const DynamicJsonDocument& doc)
{
    JsonObjectConst root = doc.as<JsonObjectConst>();
    for (JsonPairConst kv : root)
    {
        validate_json_iteration(kv);
    }
}

void TElementsDB::set_json(const DynamicJsonDocument& doc)
{
    validate_json(doc);
    //TWorker::printf("TElementsDB::set_json(..): clear()\n");
    clear();
    _add_json(doc);
}

void TElementsDB::_add_json(const DynamicJsonDocument& doc)
{
    JsonObjectConst root = doc.as<JsonObjectConst>();
    for (JsonPairConst kv : root)
    {
        //TWorker::printf("TElementsDB::_add_json(..): assign(\"%s\", \"%s\")\n", kv.key().c_str(), kv.value().as<const char *>());
        assign(
            kv.key().c_str(),
            kv.value().as<const char *>()
        );
    }
}

void TElementsDB::add_json(const DynamicJsonDocument& doc)
{
    validate_json(doc);
    _add_json(doc);
}

bool TElementsDB::is_empty(void)
{
    bool res;
    uint8_t i = 0;

    do
    {
        String s_chunk_name = get_chunk_name(i);
        prefs.begin(name_list.c_str(), false);
        uint8_t chunk = prefs.getUChar(s_chunk_name.c_str());
        prefs.end();

        res = !chunk;
    } while(++i < bitmap_size && res);

    return res;
}

String TElementsDB::get_value_id(const String& key, uint8_t *id)
{
    String res;
    prefs.begin(name.c_str(), false);
    size_t sz_data = prefs.getBytesLength(key.c_str());
    uint8_t *p_data = new uint8_t[sz_data];
    prefs.getBytes(key.c_str(), p_data, sz_data);
    prefs.end();

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

String TElementsDB::get_value(const String& key)
{
    TUtil::chk_nvs_key(key);

    String res;

    prefs.begin(name.c_str(), false);
    bool is_key = prefs.isKey(key.c_str());
    prefs.end();

    if(is_key)
    {
        res = get_value_id(key);
    }

    return res;
}
}