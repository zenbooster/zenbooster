#pragma once
#include <Arduino.h>
#include "TRingBufferInItem.h"
#include "expression.h"

namespace CalcFormula
{
using namespace RingBufferInItem;

class TCalcFormula: public TRingBufferInItem
{
  private:
    SExpression *p_ast;

    static float evaluate(SExpression *e);

  public:
    TCalcFormula(String ex) throw(String);
    ~TCalcFormula();

    static TCalcFormula *compile(const String& val) throw(String);

    float run(void);
};
}