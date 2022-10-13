#pragma once
#include "TElementsDB.h"

namespace FormulaDB
{
    using namespace ElementsDB;

    class TFormulaDB: public TElementsDB
    {
        private:
            void validate_json_iteration(JsonPairConst& kv); // может бросить исключение

        public:
            TFormulaDB();
    };
}