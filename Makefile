all : compiler

lex: frontend/sysy.l yacc
	flex -o frontend/lex.yy.cpp frontend/sysy.l && g++ -o obj/lex.o -c frontend/lex.yy.cpp -I . -I frontend -I define --std=c++17 -ll -w -g
yacc: frontend/sysy.y
	bison -d -o frontend/sysy.tab.cpp frontend/sysy.y && g++ -o obj/sysy.o -c frontend/sysy.tab.cpp -I . -I frontend -I define --std=c++17 -ll -w -g
main: define/main.cpp lex yacc
	g++ -o obj/main.o -c define/main.cpp -I . -I frontend -I define --std=c++17 -ll -w -g
ast: define/ast.cpp lex yacc
	g++ -o obj/ast.o -c define/ast.cpp -I . -I frontend -I define --std=c++17 -ll -w -g
symtab: define/symtab.cpp lex yacc
	g++ -o obj/symtab.o -c define/symtab.cpp -I . -I frontend -I define --std=c++17 -ll -w -DDEBUG -g
midend: midend/midend.cpp lex yacc
	g++ -o obj/midend.o -c midend/midend.cpp -I . -I frontend -I define --std=c++17 -ll -w -DDEBUG -g
backend: backend/backend.cpp lex yacc
	g++ -o obj/backend.o -c backend/backend.cpp -I . -I frontend -I define --std=c++17 -ll -w -DDEBUG -g
compiler : yacc lex midend main ast symtab backend
	g++ -o compiler --std=c++17 -ll obj/*.o -I . -g -w

clean :
	rm frontend/lex.yy.cpp frontend/sysy.tab.cpp frontend/sysy.tab.hpp compiler obj/*.o