#include "TCalcFormula.h"
#include "parser.h"
#include "lexer.h"

namespace CalcFormula
{
//int yyparse(SExpression **expression, yyscan_t scanner);

SExpression *getAST(const char *expr)
{
    SExpression *expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    if (yylex_init(&scanner)) {
        /* could not initialize */
        return NULL;
    }

    state = yy_scan_string(expr, scanner);

    if (yyparse(&expression, scanner)) {
        /* error parsing */
        return NULL;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return expression;
}

float TCalcFormula::evaluate(SExpression *e)
{
    switch (e->type) {
        case eVAL:
            return e->val;
        case eVAR:
            return (float)*e->p_var;
        case eDIV:
            return evaluate(e->left) / evaluate(e->right);
        case eMUL:
            return evaluate(e->left) * evaluate(e->right);
        case eSUB:
            return evaluate(e->left) - evaluate(e->right);
        case eADD:
            return evaluate(e->left) + evaluate(e->right);
        default:
            /* should not be here */
            return 0;
    }
}

TCalcFormula::TCalcFormula(String ex)
{
  reg_var("d", &delta);
  reg_var("t", &theta);
  reg_var("al", &alpha_lo);
  reg_var("ah", &alpha_hi);
  reg_var("bl", &beta_lo);
  reg_var("bh", &beta_hi);
  reg_var("gl", &gamma_lo);
  reg_var("gm", &gamma_md);
  p_ast = getAST(ex.c_str());

  if(!p_ast)
  {
    throw String("не могу скомпилировать формулу");
  }
}

TCalcFormula::~TCalcFormula()
{
    deleteExpression(p_ast);
    dict_name_id.clear();
    dict_id_ptr.clear();
}

float TCalcFormula::run(void)
{
    return evaluate(p_ast);
}

}