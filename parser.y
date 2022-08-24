%{

/*
 * Parser.y file
 * To generate the parser run: "bison Parser.y"
 */

#include "Expression.h"
#include "parser.h"
#include "lexer.h"

#define YYERROR_VERBOSE 1

// reference the implementation provided in Lexer.l
int yyerror(SExpression **expression, yyscan_t scanner, const char *msg);

%}

%code requires {
  typedef void* yyscan_t;
}

%output  "src/parser.cpp"
%defines "include/parser.h"

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { SExpression **expression }
%parse-param { yyscan_t scanner }

%union {
    float val;
    int var_id;
    SExpression *expression;
}

%token TOKEN_LPAREN   "("
%token TOKEN_RPAREN   ")"
%token TOKEN_ADD      "+"
%token TOKEN_SUB      "-"
%token TOKEN_MUL      "*"
%token TOKEN_DIV      "/"
%token <val> TOKEN_NUMBER "number"
%token <var_id> TOKEN_VAR "var"

%type <expression> expr

/* Precedence (increasing) and associativity:
   a+b+c is (a+b)+c: left associativity
   a+b*c is a+(b*c): the precedence of "*" is higher than that of "+". */
%left "+"
%left "-"
%left "*"
%left "/"

%%

input
    : expr { *expression = $1; }
    ;

expr
    : expr[L] "+" expr[R] { $$ = createOperation( eADD, $L, $R ); }
    | expr[L] "-" expr[R] { $$ = createOperation( eSUB, $L, $R ); }
    | expr[L] "*" expr[R] { $$ = createOperation( eMUL, $L, $R ); }
    | expr[L] "/" expr[R] { $$ = createOperation( eDIV, $L, $R ); }
    | "(" expr[E] ")"     { $$ = $E; }
    | "number"            { $$ = createNumber($1); }
    | "var"               { $$ = createVar($1); }
    ;

%%