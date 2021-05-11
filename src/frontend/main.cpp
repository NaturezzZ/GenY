//
// Created by Naiqian on 2021/4/25.
//

#include "define/util.h"
#include "frontend/sysy.tab.h"
#include "define/ast.h"
#include "define/util.h"
extern FILE *yyin;
//extern FILE *yyout;
int main(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'S':
                    break;
                case 'e':
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
            printf("%s\n", argv[i]);
            yyin = fopen(argv[i], "r");
        }
    }
    std::cout << "test" << std::endl;
    yyparse();
    scanTree(ASTRoot);
    return 0;
}