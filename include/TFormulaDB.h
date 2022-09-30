#pragma once
#include "TElementsDB.h"
#include "TCalcFormula.h"

namespace FormulaDB
{
    using namespace ElementsDB;
    using namespace CalcFormula;

    class TFormulaDB: public TElementsDB
    {
        public:
            TFormulaDB();

            TCalcFormula *compile(const String& val);
    };
}