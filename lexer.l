%{

/*
 * Lexer.l file
 * To generate the lexical analyzer run: "flex Lexer.l"
 */

#include "Expression.h"
#include "Parser.h"

//#include <stdio.h>
//#include <HardwareSerial.h>
extern const size_t SerialPrintf (const char *szFormat, ...);

%}

%option outfile="src/lexer.cpp" header-file="include/lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

digit [0-9]

%%

[ \r\n\t]*   { continue; /* Skip blanks. */ }
-?{digit}+(e-?{digit}+|\.{digit}+)? { yylval->val = atof(yytext); return TOKEN_NUMBER; }
[a-z]+       { 
                if (!dict_name_id.count(yytext))
                {
                    return *yytext; // raise syntax error
                }
                yylval->var_id = dict_name_id[yytext];
                return TOKEN_VAR;
             }

"/"          { return TOKEN_DIV; }
"*"          { return TOKEN_MUL; }
"-"          { return TOKEN_SUB; }
"+"          { return TOKEN_ADD; }
"("          { return TOKEN_LPAREN; }
")"          { return TOKEN_RPAREN; }

.            { continue; /* Ignore unexpected characters. */}

%%

int yyerror(SExpression **expression, yyscan_t scanner, const char *msg) {
    SerialPrintf("yyerror: %s\n", msg);
    return 0;
}