all : eeyore
eeyore : sysy.l sysy.ypp main.c
	lex sysy.l
	yacc -d sysy.y
	g++ -o eeyore lex.yy.c y.tab.c main.c -ll

clean :
	rm eeyore lex.yy.c y.tab.c y.tab.h