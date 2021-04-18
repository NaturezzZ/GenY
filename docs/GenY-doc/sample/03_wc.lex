%{
int wc = 0;		/* word count */
%}
%%

[a-zA-Z]+	{ wc++; }
\n|.		{ /* gobble up */ }

%%

int main(void)
{
	int n = yylex();
	return n;
}

int yywrap(void)
{
	printf("word count: %d\n", wc);
	return 1;
}

