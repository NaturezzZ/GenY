//
// Created by Naiqian on 2021/5/14.
//

#include "symtab.h"

std::map<ASTPtr, int> nsRootTable;
std::map<std::string, functabEntry> funcTable;
int maxNsNum = 0; // ns numbers < maxNsNum
int curNsNum = 0;
int maxtCnt = 0;
naVarTable_t naVarTable;
std::map<int, initValue> varTable; // index -> property
int breakDst = 0;
int continueDst = 0;
int maxlCnt = 0;

void symerror(const char* s){
    fprintf(stderr, "[ERROR in symbol table], %s\n", s);
    exit(2);
    abort();
}

void dout(std::string s){
#ifdef DEBUG
    cerr << s << endl;
#endif
}

bool print_flag1 = true;
bool print_flag2 = false;

void assignVar(retVal_t & obj){
    if(print_flag1) {
        cerr << "assignVar: " << std::get<0>(obj) << endl;
        auto it = funcVar.find(curFunc);
        if (it == funcVar.end()) symerror("assignVar function not found");
        it->second.push_back(obj);
    }
}

int tprintf1(const char* s){
    if(print_flag1) {
        return 0;
    }
    return -1;
}

int tprintf2(const char* s, int type){
    /*
     *  0:  SYMBOL                "=" NUMBER BinOp NUMBER
     *  1:  SYMBOL                "=" NUMBER BinOp SYMBOL
     *  2:  SYMBOL                "=" SYMBOL BinOp NUMBER
     *  3:  SYMBOL                "=" SYMBOL BinOp SYMBOL
     *  4:  SYMBOL                "=" OP SYMBOL
     *  5:  SYMBOL                "=" OP NUMBER
     *  6:  SYMBOL                "=" SYMBOL
     *  7:  SYMBOL                "=" NUMBER
     *  8:  SYMBOL "[" NUMBER "]" "=" NUMBER
     *  9:  SYMBOL "[" NUMBER "]" "=" SYMBOL
     *  10: SYMBOL "[" SYMBOL "]" "=" NUMBER
     *  11: SYMBOL "[" SYMBOL "]" "=" SYMBOL
     *  12: SYMBOL                "=" SYMBOL "[" NUMBER "]"
     *  13: SYMBOL                "=" SYMBOL "[" SYMBOL "]"
     *  14: "if" SYMBOL LOGICOP NUMBER "goto" LABEL
     *  15: "goto" LABEL
     *  16: LABEL ":"
     *  17: "param" SYMBOL
     *  18: "call" FUNCTION
     *  19: SYMBOL "=" "call" FUNCTION
     *  20: "return" NUMBER
     *  21: "return" SYMBOL
     *  22: "return";
     *  23: SYMBOL               "=" P%d
     * */
    if(print_flag2){
        auto it = funcOp.find(curFunc);
        auto typeit = funcOpType.find(curFunc);
        if(it == funcOp.end()){
            auto temp = std::vector<std::string>();
            funcOp.insert(std::make_pair(curFunc, temp));
            funcOpType.insert(std::make_pair(curFunc, std::vector<int>()));
            it = funcOp.find(curFunc);
            typeit = funcOpType.find(curFunc);
        }
        it->second.push_back(s);
        typeit->second.push_back(type);
        return 0;
    }
    return -1;
}

void switchAndCopy(retVal_t & obj, int tnum){
    char buf[100];
    switch(std::get<2>(obj)){
        case val_const_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = %d", tnum, std::get<0>(obj));
            tprintf2(buf, 7);
            break;
        case val_tvar_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d", tnum, std::get<0>(obj));
            tprintf2(buf, 6);
            break;
        case val_Tvar_:
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = T%d", tnum, std::get<0>(obj));
            tprintf2(buf, 6);
            break;
        case val_array_:
            cerr << "array!" << endl;
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = T%d[t%d]", tnum, std::get<0>(obj), std::get<1>(obj));
            tprintf2(buf, 13);
            break;
    }
}

void dectvar(int tnum){
    retVal_t assval;
    cerr << "dectvar tnum: " << tnum << endl;
    std::get<0>(assval) = tnum;
    std::get<1>(assval) = -1;
    std::get<2>(assval) = val_tvar_;
    assignVar(assval);
}

std::string getInitValString(retVal_t & val) {
    int type = std::get<2>(val);
    cerr << "initval type: " << type << endl;
    char buf[100]; memset(buf, 0, sizeof(buf));
    switch(type){
        case val_tvar_:
            sprintf(buf, "t%d", std::get<0>(val));
            return std::string(buf);
        case val_Tvar_:
            sprintf(buf, "T%d", std::get<0>(val));
            return std::string(buf);
        case val_const_:
            sprintf(buf, "%d", std::get<0>(val));
            return std::string(buf);
        case val_array_:
            sprintf(buf, "T%d[t%d]", std::get<0>(val), std::get<1>(val));
            return std::string(buf);
        default:
            symerror("retval type error");
    }
}

bool funcret = true;
bool print_flag3 = false;
bool print_flag4 = false;
bool print_flag5 = false;
std::map<std::string, std::vector<retVal_t> > funcVar;
std::map<std::string, std::vector<std::string> > funcOp;
std::map<std::string, std::vector<int>> funcOpType;
std::map<std::string, int> funcParamNum;
std::string curFunc("global_func");