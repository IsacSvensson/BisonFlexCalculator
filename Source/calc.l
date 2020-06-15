/* A simple calculator */

%option noyywrap nodefault yylineno

%{
#include "calc.hpp"
#include "calc.tab.h"
#include <string>
%}

EXP                         ([Ee][+-]?[0-9]+)

%%
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")"                         { return yytext[0]; }

 /* Comparison ops */
"<"                         { yylval.fn = 1; return CMP; }
">"                         { yylval.fn = 2; return CMP; }
"<>"                        { yylval.fn = 3; return CMP; }
"=="                        { yylval.fn = 4; return CMP; }
">="                        { yylval.fn = 5; return CMP; }
"<="                        { yylval.fn = 6; return CMP; }

 /* Keywords */
"if"                        { return IF; }
"then"                      { return THEN; }
"else"                      { return ELSE; }
"while"                     { return WHILE; }
"do"                        { return DO; }
"let"                       { return LET; }

 /* Built-in functions */
"sqrt"                      {yylval.fn = B_sqrt; return FUNC; }
"exp"                       {yylval.fn = B_exp; return FUNC; }
"log"                       {yylval.fn = B_log; return FUNC; }
"print"                     {yylval.fn = B_print; return FUNC; }

 /* Names */
[a-zA-Z][a-zA-Z0-9]*        { yylval.s = lookup(yytext); return NAME; }

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?            {
                                yylval.d = std::stod(yytext);
                                return NUMBER;
                            }

\\\n                        { printf("c> "); }
\n                          { return EOL; }
"//".*                      { }
[ \t]                       { }
.                           { yyerror("Mystery character %c\n", *yytext); }
%%

extern int yyparse();

int main(int argc, char** argv){
    printf("> ");
    return yyparse();
}
