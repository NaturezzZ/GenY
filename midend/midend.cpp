//
// Created by Naiqian on 2021/5/30.
//

#include "midend.h"

void process_global_decl(std::string funcid){
    if(funcid != "global_func"){
        miderror("global decl function id error");
    }
    auto funcVarit = funcVar.find(funcid);
    if(funcVarit == funcVar.end()) return;
    auto tVarit = funcTVar.find(funcid);
    if(tVarit == funcTVar.end()){
        std::map<TVar_t, int> tmp;
        funcTVar.insert(std::make_pair(funcid, tmp));
        tVarit = funcTVar.find(funcid);
    }
    auto funcmap = tVarit->second;
    auto declvec = funcVarit->second;
    int s = declvec.size();
    for(int i = 0; i < s; i++){
        int type = std::get<2>(declvec[i]);
        int t_index = std::get<0>(declvec[i]);
        TVar_t key; //pair<int, int>, t_type, t_index
        switch(type){
            case val_tvar_: {
                key = make_pair(tvar, t_index);
                funcmap.insert(std::make_pair(key, i));
                std::string tig = "v" + std::to_string(i) + " = 0";
                process_tigger(tig);
                break;
            }
            case val_Tvar_: {
                key = make_pair(Tvar, t_index);
                funcmap.insert(std::make_pair(key, i));
                std::string tig = "v" + std::to_string(i) + " = 0";
                process_tigger(tig);
                break;
            }
            case val_array_: {
                key = make_pair(Tarray, t_index);
                funcmap.insert(std::make_pair(key, i));
                int array_size = std::get<1>(declvec[i]);
                std::string tig = "v" + std::to_string(i) + " = malloc " + std::to_string(array_size);
                process_tigger(tig);
                break;
            }
            default:
                miderror("process decl decl vector type error");
        }
    }
    funcTVar[funcid] = funcmap;
}

void process_decl(std::string funcid){
    auto funcVarit = funcVar.find(funcid);
    if(funcVarit == funcVar.end()) return;
    auto tVarit = funcTVar.find(funcid);
    if(tVarit == funcTVar.end()){
        std::map<TVar_t, int> tmp;
        funcTVar.insert(std::make_pair(funcid, tmp));
        tVarit = funcTVar.find(funcid);
    }

    auto funcmap = tVarit->second;
    auto declvec = funcVarit->second;
    int s = declvec.size();
    int pos = 0;
    for(int i = 0; i < s; i++){
        int type = std::get<2>(declvec[i]);
        int t_index = std::get<0>(declvec[i]);

        TVar_t key; //pair<int, int>, t_type, t_index
        switch(type){
            case val_tvar_:
                key = make_pair(tvar, t_index);
                funcmap.insert(std::make_pair(key, pos));
                pos += 1;
                break;
            case val_Tvar_:
                key = make_pair(Tvar, t_index);
                funcmap.insert(std::make_pair(key, pos));
                pos += 1;
                break;
            case val_array_:
                key = make_pair(Tarray, t_index);
                funcmap.insert(std::make_pair(key, pos));
                pos += std::get<1>(declvec[i])/4;
                break;
            default:
                miderror("process decl decl vector type error");
        }
    }
    funcTVar[funcid] = funcmap;
    funcStackSize.insert(std::make_pair(funcid, pos));
}

void process_op(std::string funcid){
    auto funcOpit = funcOp.find(funcid);
    if(funcOpit == funcOp.end()){
        miderror("function has no op lines");
    }
    auto opVec = funcOpit->second;
    int s = opVec.size();
    auto opTypeVec = funcOpType.find(funcid)->second;
    std::string funchead;
    int stacksize = funcStackSize.find(funcid)->second;
    funchead = "f_" + funcid + " [" + std::to_string(funcParamNum.find(funcid)->second)+ "]";
    funchead = funchead + " [" + std::to_string(stacksize) + "]";
    process_tigger(funchead);
    int paramindex = 0;
    for(int i = 0; i < s; i++){
        dispatchEeyoreExp(opVec[i], opTypeVec[i], funcid, paramindex);
    }
    std::string funcend = "end f_" + funcid;
    process_tigger(funcend);
}

void dispatchEeyoreExp(std::string exp, int type, std::string funcid, int &paramindex){
    cerr << "[EEYORE Type: " << type << "]" << exp << endl;

    if(type <= 7 || type == 12 || type == 13 || type == 23){
        dispatchEeyoreExp_LeftSymbol(exp, type, funcid);
    }
    else if(type >= 8 && type <= 11){
        dispatchEeyoreExp_LeftArray(exp, type, funcid);
    }
    else if(type >= 20 && type <= 22){
        dispatchEeyoreExp_Return(exp, type, funcid);
    }
    else if(type == 18 || type ==19){
        dispatchEeyoreExp_Call(exp, type, funcid);
        paramindex = 0;
    }
    else if(type == 14 || type == 15 || type == 16){
        dispatchEeyoreExp_Goto(exp, type, funcid);
    }
    else if(type == 17){
        dispatchEeyoreExp_Param(exp, type, funcid, paramindex);
        paramindex++;
    }
    else{
        miderror("eeyore exp type mismatch");
    }
}

TVar_t getTVar(char a, int b){
    int type;
    if(a == 't') type = tvar;
    else if(a == 'T') type = Tvar;
    else if(a == 'n') type = numbervar;
    return std::make_pair(type, b);
}

void dispatchEeyoreExp_LeftSymbol(std::string exp, int type, std::string funcid){
    // left symbol in t1, save it into left symbol
    int num1 = 0 , num2 = 0;
    char c1, c2;
    int n1, n2;
    char cl; int nl;
    reg_t t1, t2, t3;
    t1 = std::make_pair('t', 1);
    t2 = std::make_pair('t', 2);
    t3 = std::make_pair('t', 3);
    TVar_t v1, v2, v3;
    char buf[100];
    std::string ans;
    switch(type){
        case 0:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %d %s %d", &cl, &nl, &num1, buf, &num2);
            v1 = getTVar(cl, nl);
            v2 = getTVar('n', num1);
            regLoad(v2, t2, funcid, v2);
            ans.clear();
            ans = "t1 = t2 " + std::string(buf) + std::to_string(num2);
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 1:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %d %s %c%d", &cl, &nl, &num1, buf, &c2, &n2);
            v1 = getTVar(cl, nl);
            v2 = getTVar('n', num1);
            v3 = getTVar(c2, n2);
            regLoad(v2, t2, funcid, v2);
            regLoad(v3, t3, funcid, v3);
            ans.clear();
            ans = "t1 = t2 " + std::string(buf) + " t3";
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 2:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %c%d %s %d", &cl, &nl, &c1, &n1, buf, &num2);
            v1 = getTVar(cl, nl);
            v2 = getTVar(c1, n1);
            regLoad(v2, t2, funcid, v2);
            ans.clear();
            ans = "t1 = t2 " + std::string(buf) + " " + std::to_string(num2);
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 3:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %c%d %s %c%d", &cl, &nl, &c1, &n1, buf, &c2, &n2);
            v1 = getTVar(cl, nl);
            v2 = getTVar(c1, n1);
            v3 = getTVar(c2, n2);
            regLoad(v2, t2, funcid, v2);
            regLoad(v3, t3, funcid, v3);
            ans.clear();
            ans = "t1 = t2 " + std::string(buf) + " t3";
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 4:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %s %c%d", &cl, &nl, buf, &c1, &n1);
            v1 = getTVar(cl, nl);
            v2 = getTVar(c1, n1);
            regLoad(v2, t2, funcid, v2);
            ans.clear();
            ans = "t1 = " + std::string(buf) + " t2";
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 5:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %s %d", &cl, &nl, buf, &num1);
            v1 = getTVar(cl, nl);
            ans.clear();
            ans = "t1 = " + std::string(buf) + " " + std::to_string(num1);
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 6:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %c%d", &cl, &nl, &c1, &n1);
            v1 = getTVar(cl, nl);
            v2 = getTVar(c1, n1);
            ans.clear();
            regLoad(v2, t2, funcid, v2);
            ans = "t1 = t2";
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 7:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %d", &cl, &nl, &num1);
            v1 = getTVar(cl, nl);
            ans.clear();
            ans = "t1 = " + std::to_string(num1);
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 12:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %c%d[%d]", &cl, &nl, &c1, &n1, &num1);
            v1 = getTVar(cl, nl);
            v2 = std::pair<int, int>(Tarray, n1);
            v3 = getTVar('n', num1);
            regLoad(v2, t2, funcid, v3);
            ans.clear();
            ans = "t1 = t2";
            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 13:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = %c%d[%c%d]", &cl, &nl, &c1, &n1, &c2, &n2);
            v1 = getTVar(cl, nl);
            v2 = std::pair<int, int>(Tarray, n1);
            v3 = getTVar(c2, n2);
            regLoad(v2, t1, funcid, v3);
//            ans = "t1 = t2";
//            process_tigger(ans);
            regSave(v1, t1, funcid, v1);
            break;
        }
        case 23:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = p%d", &cl, &nl, &num1);
            v1 = getTVar(cl, nl);
            reg_t a = std::pair<char, int>('a', num1);
            regSave(v1, a, funcid, v1);
            break;
        }
        default:
            miderror("dispatchEeyoreExp_LeftSymbol type mismatch");
    }
}

void dispatchEeyoreExp_LeftArray(std::string exp, int type, std::string funcid){
    cerr << "LeftArray type: " << type << endl;
    int num1 = 0 , num2 = 0;
    char c1, c2;
    int n1, n2;
    char cl; int nl;
    reg_t t1, t2, t3;
    t1 = std::make_pair('t', 1);
    t2 = std::make_pair('t', 2);
    t3 = std::make_pair('t', 3);
    TVar_t v1, v2, v3;
    char buf[100];
    std::string ans;
    switch(type){
        case 8:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d[%d] = %d", &cl, &nl, &num1, &num2);
            v1 = std::pair<int, int>(Tarray, nl);
            v2 = getTVar('n', num1);
            ans = "t1 = " + std::to_string(num2);
            process_tigger(ans);
            regSave(v1, t1, funcid, v2);
            break;
        }
        case 9:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d[%d] = %c%d", &cl, &nl, &num1, &c1, &n1);
            v1 = std::pair<int, int>(Tarray, nl);
            v2 = getTVar('n', num1);
            v3 = getTVar(c1, n1);
            regLoad(v3, t2, funcid, v3);
            ans = "t1 = t2";
            process_tigger(ans);
            regSave(v1, t1, funcid, v2);
            break;
        }
        case 10:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d[%c%d] = %d", &cl, &nl, &c1, &n1, &num1);
            v1 = std::pair<int, int>(Tarray, nl);
            v2 = getTVar(c1, n1);
            v3 = getTVar('n', num1);
            ans = "t1 = " + std::to_string(num1);
            process_tigger(ans);
            regSave(v1, t1, funcid, v2);
            break;
        }
        case 11:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d[%c%d] = %c%d", &cl, &nl, &c1, &n1, &c2, &n2);
            v1 = std::pair<int, int>(Tarray, nl);
            v2 = getTVar(c1, n1);
            v3 = getTVar(c2, n2);
            regLoad(v3, t2, funcid, v3);
            ans = "t1 = t2";
            process_tigger(ans);
            regSave(v1, t1, funcid, v2);
            break;
        }
        default:{
            miderror("dispatchEeyoreExp_LeftArray type mismatch");
        }
    }
}

void dispatchEeyoreExp_Return(std::string exp, int type, std::string funcid){
    cerr << "return type: " << type << endl;
    int num = 0;
    char cl; int nl;
    reg_t a0;
    a0 = std::make_pair('a', 0);

    TVar_t v1;
    std::string ans;
    switch(type){
        case 20:{
            sscanf(exp.c_str(), "return %d", &num);
            ans = "a0 = " + std::to_string(num);
            process_tigger(ans);
            process_tigger("return");
            break;
        }
        case 21:{
            sscanf(exp.c_str(), "return %c%d", &cl, &nl);
            v1 = getTVar(cl, nl);
            regLoad(v1, a0, funcid, v1);
            process_tigger("return");
            break;
        }
        case 22:{
            process_tigger("return");
            break;
        }
        default:{
            miderror("dispatchEeyoreExp_Return type error");
        }
    }
}

void dispatchEeyoreExp_Call(std::string exp, int type, std::string funcid){
    int num = 0;
    char cl; int nl;
    reg_t a0;
    a0 = std::make_pair('a', 0);
    char buf[100];
    TVar_t v1;
    std::string ans;
    switch(type){
        case 18:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "call %s", buf);
            ans = "call " + std::string(buf);
            process_tigger(ans);
            break;
        }
        case 19:{
            memset(buf, 0, sizeof(buf));
            sscanf(exp.c_str(), "%c%d = call %s", &cl, &nl, buf);
            v1 = getTVar(cl, nl);
            ans = "call " + std::string(buf);
            process_tigger(ans);
            regSave(v1, a0, funcid, v1);
            break;
        }
        default:{
            miderror("dispatchEeyoreExp_Call type error");
        }
    }
}

void dispatchEeyoreExp_Goto(std::string exp, int type, std::string funcid){
    int num = 0;
    char cl; int nl;
    reg_t t1 = std::make_pair('t', 1);
    TVar_t v1;
    std::string ans;
    char buf[100];
    switch(type){
        case 14:{
            sscanf(exp.c_str(), "if %c%d == 0 goto l%d", &cl, &nl, &num);
            v1 = getTVar(cl, nl);
            regLoad(v1, t1, funcid, v1);
            ans = "if t1 == x0 goto l" + std::to_string(num);
            cerr << "abcdefgfedcba" << ans << endl;
            process_tigger(ans);
            break;
        }
        case 15:{
            process_tigger(exp);
            break;
        }
        case 16:{
            process_tigger(exp);
            break;
        }
        default:{
            miderror("dispatchEeyoreExp_Param type error");
        }
    }
}

void dispatchEeyoreExp_Param(std::string exp, int type, std::string funcid, int paramindex){
    cerr << paramindex << endl;
    int num = 0;
    char cl; int nl;
    reg_t a;
    TVar_t v1;
    std::string ans;
    switch(type){
        case 17:{
            sscanf(exp.c_str(), "param %c%d", &cl, &nl);
            v1 = getTVar(cl, nl);
            a = std::make_pair('a', paramindex);
            regLoad(v1, a, funcid, v1);
            break;
        }
        default:{
            miderror("dispatchEeyoreExp_Param type error");
        }
    }
}


void regLoad(TVar_t eVar, reg_t retReg, std::string funcid, TVar_t pos){
    cerr << "load register" << endl;
    if(eVar.first == numbervar){
//        std::string ans = "load ";
//        ans = ans + std::to_string(eVar.second);
        char buf[10]; memset(buf, 0, sizeof(buf));
        buf[0] = retReg.first;
//        ans = ans + std::string(buf) + std::to_string(retReg.second);
        std::string ans = std::string(buf) + std::to_string(retReg.second) + " = " + std::to_string(eVar.second);
        process_tigger(ans);
        return;
    }

    // judge if local variable
    auto funcTVarit = funcTVar.find(funcid);
    std::map<TVar_t, int>::iterator varit;
    int index = 0;
    bool isGlobal = false;
    bool isptr = true;
    bool isarr = false;
    if(funcTVarit == funcTVar.end()){
        if(funcTVar.find("global_func") != funcTVar.end()) {
            varit = funcTVar.find("global_func")->second.find(eVar);
            if (varit == funcTVar.find("global_func")->second.end()) {}
            else {
                isptr = false;
                isGlobal = true;
            }
        }
    }
    else{
        varit = funcTVarit->second.find(eVar);
        if(varit == funcTVarit->second.end()){
            if(funcTVar.find("global_func") != funcTVar.end()) {
                varit = funcTVar.find("global_func")->second.find(eVar);
                if (varit == funcTVar.find("global_func")->second.end()) {}
                else {
                    isptr = false;
                    isGlobal = true;
                }
            }
        }
        else{
            isptr = false;
            isGlobal = false;
        }
    }

    if(isptr){
        cerr << "eVar.first == Tvar: " << (eVar.first == Tvar) << endl;
        if(eVar.first == Tarray) {
            isptr = true; isarr = false;
            TVar_t lkp = std::make_pair(Tvar, eVar.second);
            bool hasglobal = (funcTVar.find("global_func") != funcTVar.end());
            if (funcTVarit == funcTVar.end()) {
                // this function has no var, check global
                isGlobal = true;
                if (hasglobal) {
                    varit = funcTVar.find("global_func")->second.find(lkp);
                    if (varit == funcTVar.find("global_func")->second.end()) {
                        miderror("eVar not found in regLoad");
                    }
                } else {
                    miderror("eVar not found in regLoad");
                }
            } else {
                // check this function first, check global if it fails
                varit = funcTVarit->second.find(lkp);
                if (varit == funcTVarit->second.end()) {
                    // not found in this function, check it in global
                    if (!hasglobal) miderror("eVar not found in regLoad");
                    varit = funcTVar.find("global_func")->second.find(lkp);
                    if (varit == funcTVar.find("global_func")->second.end()) {
                        miderror("eVar not found in regLoad");
                    }
                    isGlobal = true;
                } else {
                    isGlobal = false;
                }
            }
        }
        else if(eVar.first == Tvar){
            isptr = false; isarr = true;
            TVar_t lkp = std::make_pair(Tarray, eVar.second);
            bool hasglobal = (funcTVar.find("global_func") != funcTVar.end());
            if (funcTVarit == funcTVar.end()) {
                // this function has no var, check global
                isGlobal = true;
                if (hasglobal) {
                    varit = funcTVar.find("global_func")->second.find(lkp);
                    if (varit == funcTVar.find("global_func")->second.end()) {
                        miderror("eVar not found in regLoad");
                    }
                } else {
                    miderror("eVar not found in regLoad");
                }
            } else {
                // check this function first, check global if it fails
                varit = funcTVarit->second.find(lkp);
                if (varit == funcTVarit->second.end()) {
                    // not found in this function, check it in global
                    if (!hasglobal) miderror("eVar not found in regLoad");
                    varit = funcTVar.find("global_func")->second.find(lkp);
                    if (varit == funcTVar.find("global_func")->second.end()) {
                        miderror("eVar not found in regLoad");
                    }
                    isGlobal = true;
                } else {
                    isGlobal = false;
                }
            }
        }
        else{
            miderror("eVar not found in regLoad");
        }
    }

    index = varit->second;
    std::string ans;
    char buf[100]; memset(buf, 0, sizeof(buf));
    if(!isGlobal) {
        switch (eVar.first) {
            case tvar:
            case Tvar: {
                memset(buf, 0, sizeof(buf));
                ans.clear();
                buf[0] = retReg.first;
                if(isarr){
                    ans = ans + "loadaddr ";
                    ans = ans + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                else {
                    ans = ans + "load ";
                    ans = ans + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                process_tigger(ans);
                break;
            }
            case Tarray: {
                //use s0
                // s0 = offset
                // loadaddr array_var retReg, retReg = array_base
                // s0 = retReg + s0, s0 = retReg + offset
                // retReg = s0[0]
                memset(buf, 0, sizeof(buf));
                ans.clear();
                switch (pos.first) {
                    case numbervar: {
                        ans = "s0 = " + std::to_string(pos.second);
//                        ans = "load " + std::to_string(pos.second) + " s0";
                        process_tigger(ans);
                        break;
                    }
                    case tvar: {
                        // load tvar into s0, use this function
                        TVar_t zero = std::make_pair(0, 0);
                        reg_t s0Reg = std::make_pair('s', 0);
                        regLoad(pos, s0Reg, funcid, zero);
                        break;
                    }
                    default: {
                        miderror("array index type error");
                    }
                }
                memset(buf, 0, sizeof(buf));
                buf[0] = retReg.first;
                ans.clear();
                if(isptr){
                    ans = "load " + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                else {
                    ans = "loadaddr " + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                process_tigger(ans);

                ans.clear();
                ans = "s0 = s0 + " + std::string(buf) + std::to_string(retReg.second);
                process_tigger(ans);

                ans.clear();
                ans = std::string(buf) + std::to_string(retReg.second) + " = s0[0]";
                process_tigger(ans);
                break;
            }
            default:
                miderror("not global variable type error");
        }
    }
    else {
        switch (eVar.first) {
            case tvar:
            case Tvar: {
                memset(buf, 0, sizeof(buf));
                ans.clear();
                buf[0] = retReg.first;
                if(isarr) {
                    ans = ans + "loadaddr v";
                    ans = ans + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                else {
                    ans = ans + "load v";
                    ans = ans + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                process_tigger(ans);
                break;
            }
            case Tarray: {
                // use s0
                // s0 = offset
                // loadaddr array_var retReg, retReg = array_base
                // s0 = retReg + s0, s0 = retReg + offset
                // retReg = s0[0]
                memset(buf, 0, sizeof(buf));
                ans.clear();
                switch (pos.first) {
                    case numbervar: {
                        ans = "s0 = " + std::to_string(pos.second);
//                        ans = "load " + std::to_string(pos.second) + " s0";
                        process_tigger(ans);
                        break;
                    }
                    case tvar: {
                        // load tvar into s0, use this function
                        TVar_t zero = std::make_pair(0, 0);
                        reg_t s0Reg = std::make_pair('s', 0);
                        regLoad(pos, s0Reg, funcid, zero);
                        break;
                    }
                    default: {
                        miderror("array index type error");
                    }
                }
                memset(buf, 0, sizeof(buf));
                buf[0] = retReg.first;
                ans.clear();
                if(isptr){
                    ans = "load v" + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                else {
                    ans = "loadaddr v" + std::to_string(index) + " ";
                    ans = ans + std::string(buf) + std::to_string(retReg.second);
                }
                process_tigger(ans);

                ans.clear();
                ans = "s0 = s0 + " + std::string(buf) + std::to_string(retReg.second);
                process_tigger(ans);

                ans.clear();
                ans = std::string(buf) + std::to_string(retReg.second) + " = s0[0]";
                process_tigger(ans);
                break;
            }
            default:
                miderror("global variable type error");
        }
    }
}

void regSave(TVar_t eVar, reg_t saveReg, std::string funcid, TVar_t pos){
    // judge if local variable
    auto funcTVarit = funcTVar.find(funcid);
    std::map<TVar_t, int>::iterator varit;
    int index = 0;
    bool isGlobal = false;
    bool isptr = true;
    if(funcTVarit == funcTVar.end()){
        if(funcTVar.find("global_func") != funcTVar.end()) {
            varit = funcTVar.find("global_func")->second.find(eVar);
            if (varit == funcTVar.find("global_func")->second.end()) {}
            else {
                isptr = false;
                isGlobal = true;
            }
        }
    }
    else{
        varit = funcTVarit->second.find(eVar);
        if(varit == funcTVarit->second.end()){
            if(funcTVar.find("global_func") != funcTVar.end()) {
                varit = funcTVar.find("global_func")->second.find(eVar);
                if (varit == funcTVar.find("global_func")->second.end()) {}
                else {
                    isptr = false;
                    isGlobal = true;
                }
            }
        }
        else{
            isptr = false;
            isGlobal = false;
        }
    }

    if(isptr){
        cerr << "eVar.first == Tarray: " << (eVar.first == Tarray) << endl;
        if(eVar.first != Tarray){
            miderror("eVar not found in regSave");
        }
        TVar_t lkp = std::make_pair(Tvar, eVar.second);
        bool hasglobal = (funcTVar.find("global_func") != funcTVar.end());
        if(funcTVarit == funcTVar.end()){
            // this function has no var, check global
            isGlobal = true;
            if(hasglobal){
                varit = funcTVar.find("global_func")->second.find(lkp);
                if(varit == funcTVar.find("global_func")->second.end()) {
                    miderror("eVar not found in regLoad");
                }
            }
            else{
                miderror("eVar not found in regLoad");
            }
        }
        else{
            // check this function first, check global if it fails
            varit = funcTVarit->second.find(lkp);
            if(varit == funcTVarit->second.end()){
                // not found in this function, check it in global
                if(!hasglobal) miderror("eVar not found in regSave");
                varit = funcTVar.find("global_func")->second.find(lkp);
                if(varit == funcTVar.find("global_func")->second.end()) {
                    miderror("eVar not found in regSave");
                }
                isGlobal = true;
            }
            else{
                isGlobal = false;
            }
        }
    }

    index = varit->second;
    std::string ans;
    char buf[100]; memset(buf, 0, sizeof(buf));
    buf[0] = saveReg.first;
    std::string saveRegStr(buf);
    saveRegStr = saveRegStr + std::to_string(saveReg.second);
    if(!isGlobal) {
        switch (eVar.first) {
            case tvar:
            case Tvar: {
                ans.clear();
                ans = "store ";
                ans = ans + saveRegStr;
                ans = ans + " " + std::to_string(index);
                process_tigger(ans);
                break;
            }
            case Tarray: {
                //use s0, s1
                // s0 = offset
                // loadaddr array_var s1, s1 = array_base
                // s0 = s1 + s0, s0 = saveReg + offset
                // s0[0] = saveReg
                memset(buf, 0, sizeof(buf));
                ans.clear();
                switch (pos.first) {
                    case numbervar: {
                        ans = "s0 = " + std::to_string(pos.second);
//                        ans = "load " + std::to_string(pos.second) + " s0";
                        process_tigger(ans);
                        break;
                    }
                    case tvar: {
                        // load tvar into s0, use this function
                        TVar_t zero = std::make_pair(0, 0);
                        reg_t s0Reg = std::make_pair('s', 0);
                        regLoad(pos, s0Reg, funcid, zero);
                        break;
                    }
                    default: {
                        miderror("array index type error");
                    }
                }
                memset(buf, 0, sizeof(buf));
                ans.clear();
                if(isptr){
                    ans = "load " + std::to_string(index) + " s1";
                }
                else {
                    ans = "loadaddr " + std::to_string(index) + " s1";
                }
                process_tigger(ans);

                ans.clear();
                ans = "s0 = s0 + s1";
                process_tigger(ans);

                ans.clear();
                ans = "s0[0] = " + saveRegStr;
                process_tigger(ans);
                break;
            }
            default:
                miderror("not global variable type error");
        }
    }
    else {
        switch (eVar.first) {
            case tvar:
            case Tvar: {
                // loadaddr VAR s0
                // s0[0] = saveReg
                ans.clear();
                ans = "loadaddr v" + std::to_string(index);
                ans = ans + " s0";
                process_tigger(ans);

                ans.clear();
                ans = "s0[0] = " + saveRegStr;
                process_tigger(ans);
                break;
            }
            case Tarray: {
                //use s0, s1
                // s0 = offset
                // loadaddr array_var s1, s1 = array_base
                // s0 = s1 + s0, s0 = saveReg + offset
                // s0[0] = saveReg
                memset(buf, 0, sizeof(buf));
                ans.clear();
                switch (pos.first) {
                    case numbervar: {
                        ans = "s0 = " + std::to_string(pos.second);
//                        ans = "load " + std::to_string(pos.second) + " s0";
                        process_tigger(ans);
                        break;
                    }
                    case tvar: {
                        // load tvar into s0, use this function
                        TVar_t zero = std::make_pair(0, 0);
                        reg_t s0Reg = std::make_pair('s', 0);
                        regLoad(pos, s0Reg, funcid, zero);
                        break;
                    }
                    default: {
                        miderror("array index type error");
                    }
                }
                memset(buf, 0, sizeof(buf));
                ans.clear();
                if(isptr){
                    ans = "load v" + std::to_string(index) + " s1";
                }
                else {
                    ans = "loadaddr v" + std::to_string(index) + " s1";
                }
                process_tigger(ans);

                ans.clear();
                ans = "s0 = s0 + s1";
                process_tigger(ans);

                ans.clear();
                ans = "s0[0] = " + saveRegStr;
                process_tigger(ans);
                break;
            }
            default:
                miderror("not global variable type error");
        }
    }
}

void miderror(const char* s){
    fprintf(stderr, "%s\n", s);
    exit(4);
}
std::map<std::string, std::map<TVar_t, int> > funcTVar; // map<function_name, map<eeyore_name, tigger_index>>
std::map<std::string, int> funcStackSize;
