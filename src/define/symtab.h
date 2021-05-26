//
// Created by Naiqian on 2021/5/14.
//

#ifndef GENY_SYMTAB_H
#define GENY_SYMTAB_H
#include <bits/stdc++.h>
#include "define/ast.h"
#include "frontend/sysy.tab.h"
typedef std::tuple<int, int, int> retVal_t;
typedef std::pair<std::string, int> naVarType; //name, curNsNum
extern std::map<ASTPtr, int> nsRootTable;

class symtabEntry {
public:
    symtabEntry() = default;
};

class functabEntry {
public:
    int paramNum;
    bool retInt;
    functabEntry() {
        paramNum = 0;
        retInt = 0;
    }
    explicit functabEntry(int n) {
        paramNum = n;
        retInt = 0;
    }
    functabEntry(const functabEntry& obj){
        paramNum = obj.paramNum;
        retInt = obj.retInt;
    }
    functabEntry & operator = (const functabEntry & obj){
        paramNum = obj.paramNum;
        retInt = obj.retInt;
        return *this;
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
        auto id = key.first;
        auto nsnum = key.second;
        ASTPtr nsroot = nullptr;
        auto nsit = nsRootTable.begin();
        while(nsit != nsRootTable.end()){
            if(nsit->second != nsnum) nsit++;
            else{
                nsroot = nsit->first;
                break;
            }
        }
        if(nsroot== nullptr) {
            std::cerr << "[ERROR] get index error" << std::endl;
            abort();
        }

        while(1) {
            if(nsRootTable.find(nsroot) == nsRootTable.end()){
                nsroot = nsroot->father;
                continue;
            }

            int thisLayerNsNum = nsRootTable.find(nsroot)->first;
            auto key = std::make_pair(id, thisLayerNsNum);
            auto it = data.find(key);
            if (it == data.end()){
                if(nsroot == ASTRoot){
                    std::cerr << "[ERROR] can not find index at root" << std::endl;
                    abort();
                }
                continue;
            }
            else {
                return it->second;
            }
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
    std::vector<retVal_t> value;
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
        value = std::vector<retVal_t>(obj.value);
    }
    initValue & operator = (const initValue& obj){
        isArray = obj.isArray;
        isInit = obj.isInit;
        isConst = obj.isConst;
        dims = std::vector<int>(obj.dims);
        value = std::vector<retVal_t>(obj.value);
        return *this;
    }
};

extern int curNsNum;
extern int maxNsNum;
extern std::map<std::string, functabEntry> funcTable;
extern naVarTable_t naVarTable;
extern std::map<int, initValue> varTable;
extern int breakDst;
extern int continueDst;
extern int maxlCnt;
void symerror(const char* s);
extern bool p_t;
extern int maxtCnt;
int tprintf1(const char* s);
int tprintf2(const char* s);
extern bool print_flag1;
extern bool print_flag2;
void switchAndCopy(retVal_t & obj, int tnum);
void dectvar(int tnum);
#endif//GENY_SYMTAB_H
