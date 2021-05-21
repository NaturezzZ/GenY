//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_TYPE_H
#define GENY_TYPE_H
#include <bits/stdc++.h>
class BaseType;
using TypePtr = std::shared_ptr<BaseType>;
using TypePtrList = std::vector<TypePtr>;

enum VAL_TYPE{
    val_const_ = 0,
    val_tvar_,
    val_array_,
    val_Tvar_,
//    val_t_var_,
//    val_T_var_,
//    val_t_array_,
//    val_T_array_,
};

enum BOPS{
    BOP_AND = 0xff1,
    BOP_OR,
    BOP_EQ,
    BOP_NE,
    BOP_GE,
    BOP_LE,
};

enum TYPES{
    TBase = 0xff1,
    TRoot,
    TCompUnit,
    TDecl,
    TDefList,
    TDef,
    TInitList,
    TConstExp,
    TNestList,
    TAddExp,
    TFuncDef,
    TFuncParam,
    TExpList,
    TBlock,
    TBlockItemList,
    TBlockItem,
    TStmt,
    TIfBlock,
    TWhileBlock,
    TExp,
    TLOrExp,
    TCond,
    TLVal,
    TNumber,
    TOp,
};

class BaseType {
public:

    //general
    int type;

    //for CompUnit
    bool withCompUnit;
    bool withDecl;
    bool withFuncDef;

    //for Decl
    bool isConstDecl;

    //for DefList
    bool isConstDefList;
    bool isDest;

    //for InitList
    bool isConstInitList;
    bool isReturnInt;
    bool withFuncFParam;

    //for FuncParam
    bool isDet;

    //for empty derivations
    bool isEmpty;

    //for stmt
    bool isBreak;
    bool isContinue;
    bool isReturn;
    bool withReturnValue;

    BaseType(){
        type = TBase;
        withCompUnit = false;
        withDecl = false;
        withFuncDef = false;
        isConstDecl = false;
        isConstDefList = false;
        isDest = false;
        isConstInitList = false;
        isReturnInt = false;
        withFuncFParam = false;
        isDet = false;
        isEmpty = false;
        isBreak = false;
        isContinue = false;
        isReturn = false;
        withReturnValue = false;

    }
};
#endif //GENY_TYPE_H
