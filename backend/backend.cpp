//
// Created by Naiqian on 2021/5/31.
//

#include "backend.h"
void process_tigger(std::string s){
    if(print_flag4){
//        fprintf(stderr, "[TIGGER]%s\n", s.c_str());
        fprintf(yyout, "%s\n", s.c_str());
    }
}

