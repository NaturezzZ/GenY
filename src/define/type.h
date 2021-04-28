//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_TYPE_H
#define GENY_TYPE_H
#include <bits/stdc++.h>
class BaseType;
using TypePtr = std::shared_ptr<BaseType>;
using TypePtrList = std::vector<TypePtr>;

enum TYPES{
    TBase = 0xff1,
    TRoot,
    TCompUnit,
    TVarDecl,

};

class BaseType {
public:

    //general
    int type;

    //for CompUnit
    bool withCompUnit;
    bool withDecl;
    bool withFuncDef;

    //for VarDecl
    BaseType(){
        type = TBase;
        withCompUnit = false;
        withDecl = false;
        withFuncDef = false;
    }
};
#endif //GENY_TYPE_H
