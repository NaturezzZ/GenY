%{
#include <stdio.h>
%}

%%
[a-z]+ printf("Lowercase word\n");
[A-Z]+ printf("Uppercase word\n");
[a-zA-Z]+ printf("Word\n");
[0-9]+ printf("Integer\n");
[0-9.]+ printf("Float\n");
";" printf("Semicolon\n");

%%

int main()
{
yylex();
return 1;
}

yywrap()
{
return 1;
}
