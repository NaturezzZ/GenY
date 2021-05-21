//
// Created by Naiqian on 2021/5/14.
//

#include "symtab.h"

std::map<ASTPtr, int> nsRootTable;
std::map<std::string, functabEntry> funcTable;
int maxNsNum = 0; // ns numbers < maxNsNum
int curNsNum = 0;
int mactCnt = 0;
naVarTable_t naVarTable;
std::map<int, initValue> varTable; // index -> property
void symerror(const char* s){
    fprintf(stderr, "[ERROR in symbol table], %s\n", s);
    abort();
}
bool print_flag1 = true;
bool print_flag2 = false;
int tprintf1(const char* s){
    if(print_flag1) {
        return fprintf(yyout, "%s", s);
    }
    return -1;
}
int tprintf2(const char* s){
    if(print_flag2){
        return fprintf(yyout, "%s", s);
    }
    return -1;
}
void switchAndCopy(retVal_t & obj, int tnum){
    char buf[100];
    switch(std::get<2>(obj)){
        case val_const_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = %d\n", tnum, std::get<0>(obj));
            tprintf2(buf);
            break;
        case val_tvar_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d\n", tnum, std::get<0>(obj));
            tprintf2(buf);
            break;
        case val_Tvar_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = T%d\n", tnum, std::get<0>(obj));
            tprintf2(buf);
            break;
        case val_array_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = T%d[t%d]\n", tnum, std::get<0>(obj), std::get<1>(obj));
            tprintf2(buf);
            break;
    }
}
void dectvar(int tnum){
    char buf[100];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "var t%d\n", tnum);
    tprintf1(buf);
}