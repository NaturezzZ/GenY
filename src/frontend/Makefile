all : front

ly : sysy.l sysy.y
	flex sysy.l
	bison -d sysy.y

front : sysy.l sysy.y main.cpp ../define/ast.cpp
	flex sysy.l
	bison -d sysy.y
	g++ -o frontend --std=c++17 -ll ../*/*.c ../*/*.cpp -I .. -g

clean :
	rm lex.yy.c sysy.tab.c sysy.tab.h frontend