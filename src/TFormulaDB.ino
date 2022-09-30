#include "TFormulaDB.h"

namespace FormulaDB
{
TFormulaDB::TFormulaDB():
    TElementsDB("formula-db")
{
}

TCalcFormula *TFormulaDB::compile(const String& val)
{
    TCalcFormula *res = NULL;

    if(val.isEmpty())
    {
      throw String("текст формулы не может быть пустым");
    }

    try
    {
      res = new TCalcFormula(val);
    }
    catch(String& e)
    {
      delete res;
      throw e;
    }
    catch(std::bad_alloc)
    {
      throw String("ошибка выделения памяти для TCalcFormula");
    }

    return res;
}
}