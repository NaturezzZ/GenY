%{
 // Part I-a, C definition
#include <stdio.h>
#include "define/ast.h"
#include "define/type.h"
#include "define/util.h"
#include <bits/stdc++.h>
#include "frontend/sysy.tab.h"

void yyerror(const char*);
//int yywrap();
int lineno = 1;
extern "C"
{
    //int yywrap(void);
    int yylex(void);
}
%}

 // Part I-b, regular definition
number	[0-9]
letter 	[a-zA-Z]
identifier	[a-zA-Z_]([a-zA-Z_0-9])*
whitespace	[ \t]



%%
 // Part II, regular expressions
"//".*		{ } //single line comment

"break"		{ return BREAK; } // wait to realize
"continue"	{ return CONTINUE; } // wait to realize
"else"		{ return ELSE; }
"int" 		{ return INT; }
"if"		{ return IF; }
"main"		{ return MAIN; }
"return"	{ return RETURN; }
"while"		{ return WHILE; }
"void"      { return VOID; }
"const"     { return CONST;}

{number}+		{ sscanf(yytext,"%d",&yylval.int_value); return INT_CONSTANT; } //???judge yytext is a legal digit
{identifier} 	{ yylval.string_value = strdup(yytext); return IDENT; }//???check type  ???free

"&&"	{ return OP_AND; }
"||"	{ return OP_OR; }
"=="	{ return OP_EQ; }
"!="	{ return OP_NE; }
"<="    { return OP_LE; }
">="    { return OP_GE; }

"!"	{ return '!'; }
"-"	{ return '-'; }
"+"	{ return '+'; }
"*"	{ return '*'; }
"/"	{ return '/'; }
"%"	{ return '%'; }
"<"	{ return '<'; }
">"	{ return '>'; }
"="	{ return '='; }
"["	{ return '['; }
"]"	{ return ']'; }

"{"	{ return '{'; }
"}"	{ return '}'; }
"("	{ return '('; }
")"	{ return ')'; }
";"	{ return ';'; }
"," { return ','; }

{whitespace}+ 	{}
"\n"	{ ++lineno; }
"\r"	{}
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