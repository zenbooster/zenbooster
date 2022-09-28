#pragma once
#include "TRingBufferInItem.h"
#include "expression.h"
#include <string>

namespace CalcFormula
{
using namespace RingBufferInItem;
using namespace std;

class TCalcFormula: public TRingBufferInItem
{
  private:
    SExpression *p_ast;

    static float evaluate(SExpression *e);

  public:
    TCalcFormula(string ex);
    ~TCalcFormula();

    float run(void);
};
}