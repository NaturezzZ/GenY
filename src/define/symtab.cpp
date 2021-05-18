//
// Created by Naiqian on 2021/5/14.
//

#include "symtab.h"

std::map<ASTPtr, int> nsRootTable;
std::map<std::string, functabEntry> funcTable;
int maxNsNum = 0; // ns numbers < maxNsNum
int curNsNum = 0;
naVarTable_t naVarTable;
std::map<int, initValue> varTable; // index -> property
void symerror(const char* s){
    fprintf(stderr, "[ERROR in symbol table], %s\n", s);
    abort();
}