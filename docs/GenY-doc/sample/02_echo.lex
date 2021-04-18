%{
#include <stdio.h>
%}
%%
he    printf("Testing HE\n");
begin printf("Started\n");
hello printf("Hello yourself!\n");
thanks printf("Your welcome\n");
end printf("Stopped\n");
bye exit(1);
%%
int main()
{
yylex();
return 1;
}
int yywrap()
{
return 1;
}
