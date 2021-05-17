%{
/* Declarations section */
#include <stdio.h>
#define YYSTYPE int
#include "parser.tab.hpp"
#include "hw3_output.hpp"
%}

%option yylineno
%option noyywrap
whitespace  ([\r\n\t ])

%%

void                                                                                                                                                return VOID;
int                                                                                                                                                 return INT;
byte                                                                                                                                                return BYTE;
b                                                                                                                                                   return B;
bool                                                                                                                                                return BOOL;
and                                                                                                                                                 return AND;
or                                                                                                                                                  return OR;
not                                                                                                                                                 return NOT;
true                                                                                                                                                return TRUE;
false                                                                                                                                               return FALSE;
return                                                                                                                                              return RETURN;
if                                                                                                                                                  return IF;
else                                                                                                                                                return ELSE;
while                                                                                                                                               return WHILE;
break                                                                                                                                               return BREAK;
continue                                                                                                                                            return CONTINUE;
switch                                                                                                                                              return SWITCH;
case                                                                                                                                                return CASE;
default                                                                                                                                             return DEFAULT;
(\:)                                                                                                                                                return COLON;
(\;)                                                                                                                                                return SC;
(\,)                                                                                                                                                return COMMA;
(\()                                                                                                                                                return LPAREN;
(\))                                                                                                                                                return RPAREN;
(\{)                                                                                                                                                return LBRACE;
(\})                                                                                                                                                return RBRACE;
(=)                                                                                                                                                 return ASSIGN;
(==|!=)                                                                                                                                             return EQ_NEQ_RELOP;
(<|>|<=|>=)                                                                                                                                         return REL_RELOP;
(\+|\-)                                                                                                                                             return ADD_SUB_BINOP;
(\*|\/)                                                                                                                                             return MUL_DIV_BINOP;
\/\/[^\r\n]*(\r|\n|\r\n)?                                                                                                                           ;
[a-zA-Z][a-zA-Z0-9]*                                                                                                                                return ID;
0|[1-9][0-9]*                                                                                                                                       return NUM;
{whitespace}                                                                                                                                        ;
\"([^\n\r\"\\]|\\[rnt"\\])+\"                                                                                                                       return STRING;
.                                                                                                                                                   {output::errorLex(yylineno); exit(0);};

%%