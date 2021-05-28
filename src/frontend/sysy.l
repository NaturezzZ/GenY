%{
 // Part I-a, C definition
#include <stdio.h>
#include "define/ast.h"
#include "define/type.h"
#include "define/util.h"
#include <bits/stdc++.h>
#include "frontend/sysy.tab.h"

void yyerror(const char*);

int lineno = 1;
extern "C"
{
    int yywrap(void);
    int yylex(void);
}
%}

 // Part I-b, regular definition
digit	[0-9]
oct_digit [0-7]
hex_digit [0-9A-Fa-f]
integer [1-9]{digit}*
oct_integer 0{oct_digit}*
hex_integer 0[xX]{hex_digit}*
number ({integer}|{oct_integer}|{hex_integer})
letter 	[a-zA-Z]
identifier	[a-zA-Z_]([a-zA-Z_0-9])*
whitespace	[ \t]
enter [\n]|[\r\n]
comment "//"[^\n]*|("/*"([^\*]|(\*)*[^\*/])*(\*)*"*/")

%%
 // Part II, regular expressions
{enter}     { lineno++; }
"break"		{ return BREAK; } // wait to realize
"continue"	{ return CONTINUE; } // wait to realize
"else"		{ return ELSE; }
"int" 		{ return INT; }
"if"		{ return IF; }
"return"	{ return RETURN; }
"while"		{ return WHILE; }
"void"      { return VOID; }
"const"     { return CONST;}
{number}+	{
				yylval.int_value = 0;
				if(yytext[0]!='0') {
					yylval.int_value = atoi(yytext);
				}
				else if (yytext[1]=='x' || yytext[1]=='X'){
					yylval.int_value = strtol(yytext, NULL, 16);
				}
				else {
					yylval.int_value = strtol(yytext, NULL, 8);
				}
				return INT_CONSTANT;
			}
{identifier} 	{   memset(yylval.string_value, 0, sizeof(yylval.string_value));
                    memcpy(yylval.string_value, yytext, yyleng);
					// fprintf(stderr, "STRING:%s\n", yylval.string_value);
					return IDENT; }
"&&"	{ return OP_AND; }
"||"	{ return OP_OR; }
"=="	{ return OP_EQ; }
"!="	{ return OP_NE; }
"<="    { return OP_LE; }
">="    { return OP_GE; }
"!"	    { return '!'; }
"-"	    { return '-'; }
"+"	    { return '+'; }
"*"	    { return '*'; }
"/"	    { return '/'; }
"%"	    { return '%'; }
"<"	    { return '<'; }
">"	    { return '>'; }
"="	    { return '='; }
"["	    { return '['; }
"]"	    { return ']'; }
"{"	    { return '{'; }
"}"	    { return '}'; }
"("	    { return '('; }
")"	    { return ')'; }
";"	    { return ';'; }
","     { return ','; }
{whitespace} 	{}
{comment}       {}
.	{ 	char msg[100];
        sprintf(msg,"unknown symbol '%c'.",*yytext);        
        yyerror(msg);
    }
%%

 // Part III, auxiliary functions
int yywrap()
{
    return 1;
}