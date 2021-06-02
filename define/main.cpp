//
// Created by Naiqian on 2021/4/25.
//

#include "define/util.h"
#include "frontend/sysy.tab.hpp"
#include "define/ast.h"
#include "define/util.h"
#include "define/symtab.h"
#include "midend/midend.h"
extern FILE *yyin;
extern FILE *yyout;
int main(int argc, char** argv){
    bool ist = false;
    bool ise = false;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'S':
                    break;
                case 't':
                    ist = true;
                    break;
                case 'e':
                    ise = true;
                    break;
                case 'o':
                    i++;
                    yyout = fopen(argv[i], "w");
                    break;
                default:
                    break;
            }
        }
        else{
            cerr << "yyin" << argv[i] << endl;
            yyin = fopen(argv[i], "r");
        }
    }
    maxNsNum = 0;
    yyparse();
    print_flag1 = true, print_flag2 = true, print_flag3 = false, print_flag4 = false;
    dispatchRoot(ASTRoot);
    print_flag3 = ise; print_flag4 = ist;
    if(print_flag3) {
        nsRootTable.clear();
        funcTable.clear();
        maxNsNum = 0; // ns numbers < maxNsNum
        curNsNum = 0;
        maxtCnt = 0;
        naVarTable.clear();
        varTable.clear(); // index -> property
        breakDst = 0;
        continueDst = 0;
        maxlCnt = 0;
        funcret = 1;
        curFunc = "global_func";
        print_flag1 = false;
        print_flag2 = false;
        print_flag3 = true;
        print_flag4 = false;
        dispatchRoot(ASTRoot);
    }
    if(print_flag4){
        nsRootTable.clear();
        funcTable.clear();
        maxNsNum = 0; // ns numbers < maxNsNum
        curNsNum = 0;
        maxtCnt = 0;
        naVarTable.clear();
        varTable.clear(); // index -> property
        breakDst = 0;
        continueDst = 0;
        maxlCnt = 0;
        funcret = 1;
        curFunc = "global_func";
        print_flag1 = false;
        print_flag2 = false;
        print_flag3 = false;
        print_flag4 = true;
        dispatchRoot(ASTRoot);
    }
    return 0;
}