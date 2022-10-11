#include "TFormulaDB.h"

namespace FormulaDB
{
using namespace CalcFormula;

void TFormulaDB::validate_json_iteration(JsonPair& kv)
{
    TElementsDB::validate_json_iteration(kv);

    String key = kv.key().c_str();
    String val = kv.value().as<const char *>();

    try
    {
        TCalcFormula::compile(val);
    }
    catch(String& e)
    {
        throw key + ": " + e;
    }
}

TFormulaDB::TFormulaDB():
    TElementsDB("formula-db")
{
}
}