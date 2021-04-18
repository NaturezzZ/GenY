/*this program recognizes a small language consisting of identifiers,
numbers, relational operators and the keywords "if", "then", "else".*/

/* regular definitions */
delim	   [ \t\n]
ws	   {delim}+
letter	   [A-Za-z]
digit	   [0-9]
id	   {letter}({letter}|{digit})*
number	   {digit}+(\.{digit}+)?(E[+\-]?{digit}+)?

%%

{ws}	 {/* no action and no return */}
if	 {printdata("keyword");}
then	 {printdata("keyword");}
else	 {printdata("keyword");}
{id}	 {printdata("identifier");}
{number} {printdata("number");}
"<"	 {printdata("relational operator");}
"<="	 {printdata("relational operator");} 
"="	 {printdata("relational operator");}
"<>"	 {printdata("relational operator");}
">"	 {printdata("relational operator");}
">="	 {printdata("relational operator");}
.        {/* no action and no return */}

%%
#include <stdio.h>
#include <stdlib.h>

printdata(char* temp) {
  printf("Seen the %s %s.\n",temp,yytext);
}

int main()
{
yylex();
return 1;
}

yywrap()
{
return 1;
}
