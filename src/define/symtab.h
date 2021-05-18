//
// Created by Naiqian on 2021/5/14.
//

#ifndef GENY_SYMTAB_H
#define GENY_SYMTAB_H
#include <bits/stdc++.h>
#include "define/ast.h"
typedef std::pair<std::string, int> naVarType; //name, curNsNum
class symtabEntry {
public:
    symtabEntry() = default;
};

class functabEntry {
public:
    int paramNum;
    functabEntry() {
        paramNum = 0;
    }
    explicit functabEntry(int n) {
        paramNum = n;
    }
    functabEntry(const functabEntry& obj){
        paramNum = obj.paramNum;
    }
};

class naVarTable_t{
public:
    std::map<naVarType, int> data;
    int maxNaVarNum;
    naVarTable_t(){
        maxNaVarNum = 0;
    }
    int getIndex(const naVarType & key){
        std::map<naVarType, int>::iterator it;
        it = data.find(key);
        if(it == data.end()) return -1;
        else{
            return (*it).second;
        }
    }
    int insert(const std::string & id, const int & nsNum){
        auto key = std::make_pair(id, nsNum);
        auto it = data.find(key);
        if(it != data.end()){
            return (*it).second;
        }
        else{
            data[key] = maxNaVarNum;
            maxNaVarNum++;
            return maxNaVarNum-1;
        }
    }
};

class initValue{
public:
    bool isArray;
    bool isInit;
    bool isConst;
    std::vector<int> value;
    std::vector<int> dims;
    initValue(){
        isArray = false;
        isInit = false;
        isConst = false;
    }
    initValue(const initValue & obj){
        isArray = obj.isArray;
        isInit = obj.isInit;
        isConst = obj.isConst;
        dims = std::vector<int>(obj.dims);
        value = std::vector<int>(obj.value);
    }
    initValue & operator = (const initValue& obj){
        isArray = obj.isArray;
        isInit = obj.isInit;
        isConst = obj.isConst;
        dims = std::vector<int>(obj.dims);
        value = std::vector<int>(obj.value);
        return *this;
    }
};

extern std::map<ASTPtr, int> nsRootTable;
extern int curNsNum;
extern int maxNsNum;
extern std::map<std::string, functabEntry> funcTable;
extern naVarTable_t naVarTable;
extern std::map<int, initValue> varTable;
void symerror(const char* s);
#endif//GENY_SYMTAB_H