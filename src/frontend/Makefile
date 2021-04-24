all : eeyore

ly : sysy.l sysy.y
	flex sysy.l
	bison -d sysy.y

eeyore : sysy.l sysy.y main.c
	flex sysy.l
	bison -d sysy.y
	g++ -o eeyore lex.yy.c sysy.tab.c main.c -ll

clean :
	rm lex.yy.c sysy.tab.c sysy.tab.h