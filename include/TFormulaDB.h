#pragma once
#include "TElementsDB.h"
#include "TCalcFormula.h"

namespace FormulaDB
{
    using namespace ElementsDB;

    class TFormulaDB: public TElementsDB
    {
        private:
            void validate_json_iteration(JsonPair& kv); // может бросить исключение

        public:
            TFormulaDB();
    };
}