//
// Created by Naiqian on 2021/4/25.
//

#include "define/util.h"
#include "frontend/sysy.tab.h"
#include "define/ast.h"
#include "define/util.h"
#include "define/symtab.h"
extern FILE *yyin;
//extern FILE *yyout;
int main(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'S':
                    break;
                case 'e':
                    yyin = fopen(argv[i], "r");
                    break;
                case 'o':
                    i++;
                    yyout = fopen(argv[i], "w");
                    break;
                default:
                    yyout = fopen("a.eeyore", "w");
                    break;
            }
        }
        else{
            printf("%s\n", argv[i]);
            yyin = fopen(argv[i], "r");
        }
    }
    yyparse();
    print_flag1 = true, print_flag2 = false;
    dispatchRoot(ASTRoot);
    print_flag1 = false, print_flag2 = true;
    nsRootTable.clear();
    funcTable.clear();
    maxNsNum = 0; // ns numbers < maxNsNum
    curNsNum = 0;
    maxtCnt = 0;
    naVarTable_t naVarTable;
    varTable.clear(); // index -> property
    breakDst = 0;
    continueDst = 0;
    maxlCnt = 0;
    dispatchRoot(ASTRoot);
    return 0;
}