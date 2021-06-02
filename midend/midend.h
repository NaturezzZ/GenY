//
// Created by Naiqian on 2021/5/30.
//

#ifndef GENY_MIDEND_H
#define GENY_MIDEND_H
#include "backend/backend.h"
#include "define/symtab.h"
#include "define/type.h"
#include "define/util.h"
#include "define/ast.h"

typedef std::pair<char, int> reg_t;
typedef std::pair<int, int> TVar_t; // type(t, T, array), t_index
extern std::map<std::string, std::map<TVar_t, int> > funcTVar; // map<function_name, map<eeyore_name, tigger_index>>
extern std::map<std::string, int> funcStackSize;


void process_global_decl(std::string funcid);

void process_decl(std::string funcid);

void process_op(std::string funcid);

void miderror(const char* s);

void dispatchEeyoreExp(std::string exp, int type, std::string funcid, int & paramindex);

void dispatchEeyoreExp_LeftSymbol(std::string exp, int type, std::string funcid);

void dispatchEeyoreExp_LeftArray(std::string exp, int type, std::string funcid);

void dispatchEeyoreExp_Return(std::string exp, int type, std::string funcid);

void dispatchEeyoreExp_Call(std::string exp, int type, std::string funcid);

void dispatchEeyoreExp_Goto(std::string exp, int type, std::string funcid);

void dispatchEeyoreExp_Param(std::string exp, int type, std::string funcid, int paramindex);

void regLoad(TVar_t eVar, reg_t retReg, std::string funcid, TVar_t pos);

void regSave(TVar_t eVar, reg_t retReg, std::string funcid, TVar_t pos);
#endif //GENY_MIDEND_H
