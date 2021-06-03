//
// Created by Naiqian on 2021/4/25.
//
#include "define/ast.h"
#include "define/symtab.h"
#include "midend/midend.h"
using namespace std;
bool hasret = false;
void attachNode(ASTPtr father, ASTPtr child){
    child->father = father;
    auto it = father->children.begin();
    while(it != father->children.end()){
        (*it)->siblings.push_back(child);
        child->siblings.push_back(*it);
        it++;
    }
    father->children.push_back(child);
}

void dispatchRoot(ASTPtr treeRoot){
    // create a global NS and step into
    std::map<ASTPtr, int>::iterator it;
    if(print_flag4){
        std::string funcid = "global_func";
        process_global_decl(funcid);
    }
    if(print_flag1) {
        curFunc = "global_func";
        std::vector<retVal_t> tmp;
        tmp.clear();
        funcVar.insert(std::make_pair(curFunc, tmp));
    }
    if(print_flag3){
        auto it = funcVar.find(std::string("global_func"));
        auto vecit = it->second.begin();
        char buf[100];
        while(vecit != it->second.end()){
            switch(std::get<2>(*vecit)){
                case val_tvar_:{
                    fprintf(yyout, "var t%d\n", std::get<0>(*vecit));
                    break;
                }
                case val_Tvar_:{
                    fprintf(yyout, "var T%d\n", std::get<0>(*vecit));
                    break;
                }
                case val_array_:{
                    fprintf(yyout, "var %d T%d\n", std::get<1>(*vecit), std::get<0>(*vecit));
                    break;
                }
                default:
                    symerror("var declaration type error");
            }
            vecit++;
        }
    }
    it = nsRootTable.find(treeRoot);
    if(it != nsRootTable.end()) {
        curNsNum = it->second;
        asterr("layer number is not 0, at root");
    }
    else {
        curNsNum = maxNsNum; // enter global NS
        nsRootTable.insert(std::pair<ASTPtr , int>(treeRoot, curNsNum));
        maxNsNum++;
    }
    auto node = (RootAST *) treeRoot;
    dispatchCompUnit(node->children[0]);
}

void dispatchCompUnit(ASTPtr treeRoot){
    // in global NS
    auto node = (CompUnitAST *) treeRoot;
    if (node->ast_type()->withCompUnit){
        if (node->ast_type()->withDecl) {
            // CompUnit -> CompUnit Decl
            dispatchCompUnit(node->children[0]);
            dispatchDecl(node->children[1]);
        }
        else {
            // CompUnit -> CompUnit FuncDef
            dispatchCompUnit(node->children[0]);
            dispatchFuncDef(node->children[1]);
            auto p = (FuncDefAST*)(node->children[1]);
            std::string id = p->id;
            //check whether function is redefined
        }
    }
    else {
        if (node->ast_type()->withDecl) {
            // CompUnit -> Decl
            // with no compunit anymore
            // add entries directly into global ast
            dispatchDecl(node->children[0]);
        }
        else if (node->ast_type()->withFuncDef){
            // CompUnit -> FuncDef
            dispatchFuncDef(node->children[0]);
            auto p = (FuncDefAST*)(node->children[0]);
            std::string id = p->id;
        }
    }
}

void dispatchDecl(ASTPtr treeRoot) {
    if(print_flag4){
        return;
    }
    auto node = (DeclAST*) treeRoot;
    auto child = treeRoot->children[0];
    if(node->ast_type()->isConstDecl) {
        // Decl -> ConstDecl
        dispatchConstDefList(child->children[0]);
    }
    else {
        // Decl -> VarDecl
        dispatchDefList(child->children[0]);
    }
}

void dispatchConstDefList(ASTPtr treeRoot){
    auto node = (DeclAST*)treeRoot;
    int s = node->children.size();
    if(s == 1){
        dispatchConstDef(node->children[0]);
    }
    else if(s == 2){
        dispatchConstDefList(node->children[0]);
        dispatchConstDef(node->children[1]);
    }
    else{
        symerror("dispatchConstDefList children size error");
    }
}

void dispatchConstDef(ASTPtr treeRoot){
    auto node = (DefAST*)treeRoot;
    int childsize = node->children.size();
    initValue val;
    auto id = node->id;
    if(childsize == 1){
        // ConstDef -> IDENT '=' ConstInitVal
        val.isConst = true;
        val.isArray = false;
        val.isInit = true;
        auto initchild = node->children[0];
        std::vector<retVal_t> initval;
        dispatchConstInitVal(initchild, initval);
        val.value = initval;
        cerr << "id: " << id << "curNsNum: " << curNsNum << endl;
        int varnum = naVarTable.insert(id, curNsNum);
        auto it = varTable.find(varnum);
        if(it != varTable.end()) symerror("duplicate define");
        varTable.insert(std::make_pair(varnum, val));
        retVal_t assignVal;
        std::get<0>(assignVal) = varnum;
        std::get<1>(assignVal) = -1;
        std::get<2>(assignVal) = val_Tvar_;
        assignVar(assignVal);
    }
    else if(childsize == 2){
        // ConstDef -> IDENT Dimensions_list '=' ConstInitVal
        val.isConst = true;
        val.isArray = true;
        val.isInit = true;
        auto dimchild = node->children[0];
        auto initchild = node->children[1];
        auto dimtmp = dispatchDimensionsList(dimchild);
        std::vector<int> dims;
        int factor = 1;
        for(int i = 0; i < dimtmp.size(); i++){
            if(std::get<2>(dimtmp[i])!=val_const_) symerror("dispatchConstDef dimension not const");
            dims.push_back(std::get<0>(dimtmp[i]));
            factor*=std::get<0>(dimtmp[i]);
        }
        std::vector<retVal_t> initsrc;
        std::vector<retVal_t> initdst;
        auto initpattern = dispatchConstInitVal(initchild, initsrc);
        process_array(initsrc, initdst, initpattern, dims);
        val.dims = dims;
        val.value = initdst;
        int varnum = naVarTable.insert(id, curNsNum);
        auto it = varTable.find(varnum);
        if(it != varTable.end()) symerror("duplicate define");
        varTable.insert(std::make_pair(varnum, val));
        retVal_t assignVal;
        std::get<0>(assignVal) = varnum;
        std::get<1>(assignVal) = factor*4;
        std::get<2>(assignVal) = val_array_;
        assignVar(assignVal);
    }
    else{
        symerror("dispatchConstDef children size error");
    }
}

void dispatchDefList(ASTPtr treeRoot) {
    auto node = (DefListAST*) treeRoot;
    if(node->ast_type()->isDest){
        // DefList -> Def
        cerr << "Dest, DefList children number: " << node->children.size() << endl;
        auto child = node->children[0];
        dispatchDef(child);
    }
    else{
        // DefList -> Deflist ',' Def
        cerr << "DefList children number: " << node->children.size() << endl;
        auto childList = node->children[0];
        auto childDef = node->children[1];
        dispatchDefList(childList);
        dispatchDef(childDef);
    }
}

void dispatchDef(ASTPtr treeRoot) {
    // this function add definition and its initial value into target
    auto node = (DefAST*)treeRoot;
    auto id = node->id;
    if(node->isArray) {
        //is array
        auto dimchild = node->children[0];
        auto dimstmp = dispatchDimensionsList(dimchild);
        std::vector<int> dims;
        int varindex = 0;
        std::vector<retVal_t> inittmpval;
        int factor = 1;
        for (int i = 0; i < dimstmp.size(); i++) {
            if (std::get<2>(dimstmp[i]) != val_const_) {
                symerror("dimension list not const");
            }
            factor *= std::get<0>(dimstmp[i]);
            dims.push_back(std::get<0>(dimstmp[i]));
        }
        if (node->withInitVal) {
            std::vector<retVal_t> initsrc;
            std::vector<retVal_t> initdst;
            auto initchild = node->children[1];
            auto initpattern = dispatchInitVal(initchild, initsrc);
            dout(initpattern);
            process_array(initsrc, initdst, initpattern, dims);
            for(int i = 0; i < factor; i++) inittmpval.push_back(initdst[i]);
            initValue val;
            val.isInit = 1; val.isArray = 1; val.dims = dims; val.value = initdst;
            int varnum = naVarTable.insert(id, curNsNum);
            auto it = varTable.find(varnum);
            varindex = varnum;
            if(it != varTable.end()) symerror("duplicate define");
            varTable.insert(std::make_pair(varnum, val));
            retVal_t assignVal;
            std::get<0>(assignVal) = varnum;
            std::get<1>(assignVal) = factor*4;
            std::get<2>(assignVal) = val_array_;
            assignVar(assignVal);
        }
        else{
            std::vector<retVal_t> initdst;
            retVal_t tmpzero;
            std::get<0>(tmpzero) = 0;
            std::get<1>(tmpzero) = -1;
            std::get<2>(tmpzero) = val_const_;
            for(int i = 0; i < factor; i++) {
                initdst.push_back(tmpzero);
                inittmpval.push_back(tmpzero);
            }
            initValue val;
            val.isInit = 0; val.isArray = 1; val.dims = dims; val.value = initdst;
            int varnum = naVarTable.insert(id, curNsNum);
            varindex = varnum;
            auto it = varTable.find(varnum);
            if(it != varTable.end()) symerror("duplicate define");
            varTable.insert(std::make_pair(varnum, val));
            retVal_t assignVal;
            std::get<0>(assignVal) = varnum;
            std::get<1>(assignVal) = factor*4;
            std::get<2>(assignVal) = val_array_;
            assignVar(assignVal);
        }
        if(curNsNum == 0) return;
        char buf[100]; memset(buf, 0, sizeof(buf));
        if(node->withInitVal) {
            for (int i = 0; i < factor; i++) {
                int tnum = maxtCnt;
                maxtCnt++;
                retVal_t assignVal;
                std::get<0>(assignVal) = tnum;
                std::get<1>(assignVal) = -1;
                std::get<2>(assignVal) = val_tvar_;
                assignVar(assignVal);
                switchAndCopy(inittmpval[i], tnum);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "T%d[%d] = t%d", varindex, i * 4, tnum);
                tprintf2(buf, 9);
            }
        }
    }
    else{
        int varindex;
        retVal_t inittmpval;
        if (node->withInitVal) {
            auto initchild = node->children[0];
            std::vector<retVal_t> initval;
            dispatchInitVal(initchild, initval);
            inittmpval = initval[0];
            initValue val;
            val.isInit = 1; val.isArray = 0; val.value = initval;
            int varnum = naVarTable.insert(id, curNsNum);
            varindex = varnum;
            auto it = varTable.find(varnum);
            if(it != varTable.end()) symerror("duplicate define");
            varTable.insert(std::make_pair(varnum, val));
            retVal_t assignVal;
            std::get<0>(assignVal) = varnum;
            std::get<1>(assignVal) = -1;
            std::get<2>(assignVal) = val_Tvar_;
            assignVar(assignVal);
        }
        else {
            retVal_t zerotmp;
            std::get<0>(zerotmp) = 0;
            std::get<1>(zerotmp) = -1;
            std::get<2>(zerotmp) = val_const_;
            initValue val;
            val.isInit = 0; val.isArray = 0; val.value.push_back(zerotmp);
            inittmpval = zerotmp;
            int varnum = naVarTable.insert(id, curNsNum);
            varindex = varnum;
            auto it = varTable.find(varnum);
            if(it != varTable.end()) symerror("duplicate define");
            varTable.insert(std::make_pair(varnum, val));
            retVal_t assignVal;
            std::get<0>(assignVal) = varnum;
            std::get<1>(assignVal) = -1;
            std::get<2>(assignVal) = val_Tvar_;
            assignVar(assignVal);

        }
        if(curNsNum == 0) return;
        int tnum = maxtCnt; maxtCnt++;
        retVal_t assignVal;
        std::get<0>(assignVal) = tnum;
        std::get<1>(assignVal) = -1;
        std::get<2>(assignVal) = val_tvar_;
        assignVar(assignVal);
        switchAndCopy(inittmpval, tnum);
        char buf[100]; memset(buf, 0, sizeof(buf));
        sprintf(buf, "T%d = t%d", varindex, tnum);
        tprintf2(buf, 6);
    }
}

std::vector<retVal_t> dispatchDimensionsList(ASTPtr treeRoot){
    // dimensions list must be const
    auto node = (DimensionsListAST*)treeRoot;
    int childnum = static_cast<int>(node->children.size());
    std::vector<retVal_t> res;
    if(childnum == 1){
        // leaf
        auto constexp = dispatchConstExp(node->children[0]);
        res.push_back(constexp);
        return res;
    }
    else if(childnum == 2){
        // not leaf
        auto constexplist = dispatchDimensionsList(node->children[0]);
        auto constexp = dispatchConstExp(node->children[1]);
        int s = constexplist.size();
        for(int i = 0; i < s; i++){
            res.push_back(constexplist[i]);
        }
        res.push_back(constexp);
        return res;
    }
}

std::string dispatchInitVal(ASTPtr treeRoot, std::vector<retVal_t>& target) {
    auto node = (NestListAST*)treeRoot;
    int s = node->children.size();
    if(s == 0){
        // InitVal -> '{' '}'
        return std::string("{}");
    }
    else if(s == 1){
        auto child = node->children[0];
        if(child->ast_type()->type == TExp){
            // InitVal -> Exp
            auto val = dispatchExp(child);
            target.push_back(val);
            return std::string("n");
        }
        else {
            // InitVal -> '{' InitVal_list '}'
            return std::string("{") + dispatchInitList(child, target) + std::string("}");
        }
    }
}

std::string dispatchInitList(ASTPtr treeRoot, std::vector<retVal_t>& target) {
    auto node = (NestListAST*) treeRoot;
    int s = node->children.size();
    if(s == 1){
        // InitVal_list -> InitVal
        return dispatchInitVal(node->children[0], target);
    }
    else if(s == 2){
        // Initval_list -> InitVal_list ',' InitVal
        auto s1 = dispatchInitList(node->children[0], target);
        auto s2 = dispatchInitVal(node->children[1], target);
        return s1 + s2;
    }
    else{
        symerror("In dispatchInitList, children size fault");
    }
}

std::string dispatchConstInitVal(ASTPtr treeRoot, std::vector<retVal_t>& target){
    auto node = (NestListAST*)treeRoot;
    int s = node->children.size();
    cerr << "children size: " << s << endl;
    if(s == 0){
        return std::string("{}");
    }
    else if(s == 1){
        auto child = node->children[0];
        if(child->ast_type()->type == TConstExp){
            cerr << "hit" << endl;
            auto val = dispatchConstExp(child);
            if(std::get<2>(val) != val_const_) symerror("In dispatchConstInitVal, ConstExp is not const");
            target.push_back(val);
            return std::string("n");
        }
        else{
            return std::string("{") + dispatchConstInitList(child, target) + std::string("}");
        }
    }
    else{
        symerror("In dispatchConstInitVal, child type fault");
    }
}

std::string dispatchConstInitList(ASTPtr treeRoot, std::vector<retVal_t>& target) {
    auto node = (NestListAST*)treeRoot;
    int s;
    s = node->children.size();
    initValue val;
    if(s == 1){
        // ConstInitVal_list -> ConstInitVal
        return dispatchConstInitVal(node->children[0], target);
    }
    else if(s == 2){
        // ConstInitVal_list -> ConstInitVal_list ',' ConstInitVal
        auto s1 = dispatchConstInitList(node->children[0], target);
        auto s2 = dispatchConstInitVal(node->children[1], target);
        return s1 + s2;
    }
    else{
        symerror("dispatchInitList children number error");
    }
}

retVal_t dispatchConstExp(ASTPtr treeRoot){
    auto node = (ConstExpAST*)treeRoot;
    auto addexp = dispatchAddExp(node->children[0]);
    if(std::get<2>(addexp)!=val_const_){
        symerror("Add expression in const expression is not const");
    }
    return addexp;
}

retVal_t dispatchAddExp(ASTPtr treeRoot) {
    char buf[100];
    auto node = (ExpAST*) treeRoot;
    int size = static_cast<int>(node->children.size());
    retVal_t ret;
    if(size == 1){
        auto mul = dispatchMulExp(node->children[0]);
        return mul;
    }
    else if(size == 3){
        auto mul = dispatchAddExp(node->children[0]);
        auto add = dispatchMulExp(node->children[2]);
        auto addOp = (OpAST*)(node->children[1]);

        if(std::get<2>(mul) == val_const_ && std::get<2>(add) == val_const_){
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_const_;
            switch(addOp->op){
                case '+':
                    std::get<0>(ret) = std::get<0>(mul) + std::get<0>(add);
                    break;
                case '-':
                    std::get<0>(ret) = std::get<0>(mul) - std::get<0>(add);
                    break;
                default:
                    symerror("unrecognized add operator in dispatchAddExp");
            }
            return ret;
        }
        else {
            int tnum1 = maxtCnt;
            maxtCnt++;
            int tnum2 = maxtCnt;
            maxtCnt++;
            int restnum = maxtCnt;
            maxtCnt++;

            std::get<0>(ret) = restnum;
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_tvar_;

            dectvar(tnum1);
            dectvar(tnum2);
            dectvar(restnum);
            switchAndCopy(mul, tnum1);
            switchAndCopy(add, tnum2);
            switch (addOp->op) {
                case '+':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d + t%d", restnum, tnum1, tnum2);
                    tprintf2(buf, 3);
                    break;
                case '-':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d - t%d", restnum, tnum1, tnum2);
                    tprintf2(buf, 3);
                    break;
                default:
                    symerror("unrecognized add operator in dispatchAddExp");
            }
            return ret;
        }
    }
}

retVal_t dispatchMulExp(ASTPtr treeRoot) {
    dout("MulExp");
    char buf[100];
    auto node = (ExpAST *) treeRoot;
    int size = static_cast<int>(node->children.size());
    if (size == 1) {
        auto unary = dispatchUnaryExp(node->children[0]);
        return unary;
    } else if (size == 3) {
        // MulExp -> MulExp Mul_Div_Mod UnaryExp
        auto ptr1 = node->children[0];
        auto ptr3 = node->children[2];
        auto mul = dispatchMulExp(ptr1);
        auto unary = dispatchUnaryExp(ptr3);
        auto op = ((OpAST *) (node->children[1]))->op;
        retVal_t ret;
        if (std::get<2>(mul) == val_const_ && std::get<2>(unary) == val_const_) {
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_const_;
            switch (op) {
                case '*':
                    std::get<0>(ret) = std::get<0>(mul) * std::get<0>(unary);
                    return ret;
                case '/':
                    std::get<0>(ret) = std::get<0>(mul) / std::get<0>(unary);
                    return ret;
                case '%':
                    std::get<0>(ret) = std::get<0>(mul) % std::get<0>(unary);
                    return ret;
                default:
                    symerror("no correct op in dispatchMulExp");
            }
        } else {
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_tvar_;
            int tnum1 = maxtCnt;
            maxtCnt++;
            int tnum2 = maxtCnt;
            maxtCnt++;
            int tnum3 = maxtCnt;
            maxtCnt++;
            dectvar(tnum1);
            dectvar(tnum2);
            dectvar(tnum3);
            switchAndCopy(mul, tnum1);
            switchAndCopy(unary, tnum2);
            switch (op) {
                case '*':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d * t%d", tnum3, tnum1, tnum2);
                    tprintf2(buf, 3);
                    break;
                case '/':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d / t%d", tnum3, tnum1, tnum2);
                    tprintf2(buf, 3);
                    break;
                case '%':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d %c t%d", tnum3, tnum1, '%', tnum2);
                    tprintf2(buf, 3);
                    break;
                default:
                    symerror("dispatch MulExp");
            }
            std::get<0>(ret) = tnum3;
            return ret;
        }
    }
}

retVal_t dispatchUnaryExp(ASTPtr treeRoot) {
    dout("Unary");
    auto node = (ExpAST*) treeRoot;
    cerr << "id:" << node->id << endl;
    int size = static_cast<int>(node->children.size());
    if(size == 0){
        // UnaryExp -> Ident '(' ')'
        char buf[100];
        if(funcret) {
            int tnum = maxtCnt;
            maxtCnt++;
            dectvar(tnum);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = call f_%s", tnum, node->id.c_str());
            tprintf2(buf, 19);
            retVal_t ret;
            std::get<2>(ret) = val_tvar_;
            std::get<1>(ret) = -1;
            std::get<0>(ret) = tnum;
            return ret;
        }
        else {
            int tnum = maxtCnt; maxtCnt++;
            dectvar(tnum);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = 0", tnum);
            tprintf2(buf, 7);
            if(node->id == "starttime" || node->id == "stoptime"){
                // int pnum = maxtCnt; maxtCnt++;
                // dectvar(pnum);
                // memset(buf, 0, sizeof(buf));
                // sprintf(buf, "t%d = %d", pnum, node->lineno);
                // tprintf2(buf, 7);
                // memset(buf, 0, sizeof(buf));
                // sprintf(buf, "param t%d", pnum);
                // tprintf2(buf, 17);
                // memset(buf, 0, sizeof(buf));
                node->id = "_sysy_" + node->id;
            }
            sprintf(buf, "call f_%s", node->id.c_str());
            tprintf2(buf, 18);
            retVal_t ret;
            std::get<2>(ret) = val_tvar_;
            std::get<1>(ret) = -1;
            std::get<0>(ret) = tnum;
            return ret;
        }
    }
    auto child0 = (NumberAST*)node->children[0];
    if(size == 1 && child0->ast_type()->type == TExp){
        // UnaryExp -> Primary
        auto ptr1 = (ExpAST*) (node->children[0]);
        auto primary = dispatchPrimaryExp(ptr1);
        return primary;
    }
    else if(size == 1 && child0->ast_type()->type == TFuncParam){
        // UnaryExp -> IDENT '(' FuncRParams ')'
        bool lastfuncret = funcret;
        funcret = true;
        auto retVals = dispatchFuncRParamList(node->children[0]);
        funcret = lastfuncret;
        int paramNum = retVals.size();
        char buf[100];
        for(int i = 0; i < paramNum; i++){
            cerr << "param type: " << std::get<2>(retVals[i]) << endl;
        }
        for(int i = 0; i < paramNum; i++){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "param t%d", std::get<0>(retVals[i]));
            tprintf2(buf, 17);
        }

        if(funcret) {
            int tnum = maxtCnt;
            maxtCnt++;
            dectvar(tnum);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = call f_%s", tnum, node->id.c_str());
            tprintf2(buf, 19);
            retVal_t val;
            std::get<0>(val) = tnum;
            std::get<1>(val) = -1;
            std::get<2>(val) = val_tvar_;
            return val;
        }
        else {
            int tnum = maxtCnt; maxtCnt++;
            dectvar(tnum);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = 0", tnum);
            tprintf2(buf, 7);
            sprintf(buf, "call f_%s", node->id.c_str());
            tprintf2(buf, 18);
            retVal_t ret;
            std::get<2>(ret) = val_tvar_;
            std::get<1>(ret) = -1;
            std::get<0>(ret) = tnum;
            return ret;
        }
    }
    else if(size == 2){
        // UnaryExp -> UnaryOp UnaryExp
        auto ptr1 = (OpAST*) (node->children[0]);
        auto ptr2 = (ExpAST*) (node->children[1]);
        auto unary = dispatchUnaryExp(ptr2);
        switch (std::get<2>(unary)){
            case val_const_: {
                int ans = std::get<0>(unary);
                switch (ptr1->op) {
                    case '+':
                        ans = ans;
                        break;
                    case '-':
                        ans = -ans;
                        break;
                    case '!':
                        ans = !ans;
                        break;
                }
                std::get<0>(unary) = ans;
                std::get<1>(unary) = -1;
                std::get<2>(unary) = val_const_;
                return unary;
            }
            case val_array_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                dectvar(tnum);

                int tmp = 0;
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]", tnum, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf, 13);
                        break;
                    case '-':
                        tmp = maxtCnt; maxtCnt++;
                        dectvar(tmp);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]", tmp, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf, 13);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = - t%d", tnum, tmp);
                        tprintf2(buf, 4);
                        break;
                    case '!':
                        tmp = maxtCnt; maxtCnt++;
                        dectvar(tmp);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]", tmp, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf, 13);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = ! t%d", tnum, tmp);
                        tprintf2(buf, 4);
                        break;
                }
                std::get<0>(unary) = tnum;
                std::get<1>(unary) = -1;
                std::get<2>(unary) = val_tvar_;
                return unary;
            }
            case val_tvar_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                dectvar(tnum);
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = t%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 6);
                        break;
                    case '-':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = - t%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 4);
                        break;
                    case '!':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = ! t%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 4);
                        break;
                }
                std::get<0>(unary) = tnum;
                std::get<1>(unary) = -1;
                std::get<2>(unary) = val_tvar_;
                return unary;
            }
            case val_Tvar_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                dectvar(tnum);
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 6);
                        break;
                    case '-':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = - T%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 4);
                        break;
                    case '!':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = ! T%d", tnum, std::get<0>(unary));
                        tprintf2(buf, 4);
                        break;
                }
                std::get<0>(unary) = tnum;
                std::get<1>(unary) = -1;
                std::get<2>(unary) = val_tvar_;
                return unary;
            }
            default:
                symerror("unary type error");
        }
    }
    else{
        symerror("dispatch UnaryExp err");
    }
}

retVal_t dispatchPrimaryExp(ASTPtr treeRoot) {
    auto node = (PrimaryExpAST*)treeRoot;
    auto child = (node->children[0]);
    switch(node->type){
        case 0: {
            auto exp = dispatchExp(child);
            return exp;
        }
        case 1: {
            auto lval = dispatchLVal(child);
            return lval;
        }
        case 2: {
            retVal_t ret;
            auto number = (NumberAST*)child;
            std::get<0>(ret) = number->value;
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_const_;
            return ret;
        }
        default:
            symerror("unrecognized type in dispatchPrimaryExp");
    }
}

std::vector<retVal_t> dispatchExpList(ASTPtr treeRoot) {
    auto node = (ExpListAST*)treeRoot;
    std::vector<retVal_t> res;
    int s = (node->children).size();
    if(s == 0){
        return res;
    }
    auto exp = dispatchExp(node->children[0]);
    auto explist = dispatchExpList(node->children[1]);
    res.push_back(exp);
    int explistsize = explist.size();
    for(int i = 0; i < explistsize; i++){
        res.push_back(explist[i]);
    }
    return res;
}

retVal_t dispatchExp(ASTPtr treeRoot) {
    auto node = (ExpAST*)treeRoot;
    auto addexp = dispatchAddExp(node->children[0]);
    return addexp;
}

retVal_t dispatchLVal(ASTPtr treeRoot) {
    /*
     * return value, position, type
     * if LVal is a constant, return (value, -1, const_type)
     * if LVal is a non-const var, return (number, -1, var_type)
     * if LVal is a non-const array, return (number, position, array_type)
     */
    auto node = (LValAST*) treeRoot;
    auto id = node->id;
    auto key = std::make_pair(id, curNsNum);
    int index = naVarTable.getIndex(key);
    retVal_t ret;
    if(index == -1){
        //not found
        symerror("LVal Ident not found in naVarTable");
    }
    auto it = varTable.find(index);
    if(it == varTable.end()){
        symerror("LVal Ident not found in varTable");
    }
    auto property = it->second;
    if(property.isArray){
        std::vector<retVal_t> explist(dispatchExpList(node->children[0]));
        int s = static_cast<int>(explist.size());
        cerr << "ExpSize=" << s << endl;
        if(s == 0) {
            // parameter for an array
            std::get<0>(ret) = index;
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_Tvar_;
            return ret;
        }
        else if(s != property.dims.size()){
            // parameter for some dimension
            int factor = 1;
            for(int i = 0; i < property.dims.size()-s; i++){
                factor *= property.dims[s+i];
            }
            int tempconst = maxtCnt;
            maxtCnt++;
            dectvar(tempconst);
            char buf[100];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = %d", tempconst, 4*factor);
            tprintf2(buf, 7);

            int t_index_base = maxtCnt;
            maxtCnt++;
            dectvar(t_index_base);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = 0", t_index_base);
            tprintf2(buf, 7);

            for(int i = 0; i < s; i++){
                auto exp = explist[i];
                int tnum = maxtCnt;
                maxtCnt++;
                dectvar(tnum);
                int tmp = 0;
                switch (std::get<2>(exp)){
                    case val_const_: {
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = %d", tnum, std::get<0>(exp));
                        tprintf2(buf, 7);
                        break;
                    }
                    case val_tvar_: {
                        tmp = std::get<0>(exp);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = t%d", tnum, tmp);
                        tprintf2(buf, 6);
                        break;
                    }
                    case val_Tvar_: {
                        tmp = std::get<0>(exp);
                        // native var
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d", tnum, tmp);
                        tprintf2(buf, 6);
                        break;
                    }
                    case val_array_: {
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]", tnum, std::get<0>(exp), std::get<1>(exp));
                        tprintf2(buf, 13);
                        break;
                    }
                    default: symerror("array non const type error");
                }
                cerr << "property dims size" << endl;
                cerr << property.dims.size() << endl;
                int factor = 1;
                for(int j = i+1; j < s; j++){
                    factor *= property.dims[j];
                }
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d * %d", tnum, tnum, factor);
                tprintf2(buf, 2);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d + t%d", t_index_base, t_index_base, tnum);
                tprintf2(buf, 3);
            }

            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d * t%d", tempconst, t_index_base, tempconst);
            tprintf2(buf, 3);

            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = T%d + t%d", tempconst, index, tempconst);
            tprintf2(buf, 3);

            std::get<0>(ret) = tempconst;
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_tvar_;
            return ret;
        }
        bool isConst = true;
        for(int i = 0; i < s; i++){
            if(std::get<2>(explist[i]) != val_const_) isConst = false;
        }

        if(isConst){
            int cnt = 0;
            for(int i = 0; i < s; i++){
                if(std::get<2>(explist[i]) != val_const_) symerror("LVal array index not const");
                int factor = 1;
                for(int j = i+1; j < s; j++){
                    factor *= property.dims[j];
                }
                cnt += factor*std::get<0>(explist[i]);
            }

            if(property.isConst){
                std::get<0>(ret) = std::get<0>(property.value[cnt]);
                std::get<1>(ret) = -1;
                std::get<2>(ret) = val_const_;
            }
            else {
                int tempconst = maxtCnt;
                maxtCnt++;
                dectvar(tempconst);
                char buf[100];
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = %d", tempconst, 4*cnt);
                tprintf2(buf, 7);
                std::get<0>(ret) = index;
                std::get<1>(ret) = tempconst;
                std::get<2>(ret) = val_array_;
            }
        }
        else {
            // exist non const dimensions
            int t_index_base = maxtCnt;
            maxtCnt++;
            char buf[100];
            dectvar(t_index_base);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = 0", t_index_base);
            tprintf2(buf, 7);

            for(int i = 0; i < s; i++){
                auto exp = explist[i];
                int tnum = maxtCnt;
                maxtCnt++;
                dectvar(tnum);
                int tmp = 0;
                switch (std::get<2>(exp)){
                    case val_const_: {
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = %d", tnum, std::get<0>(exp));
                        tprintf2(buf, 7);
                        break;
                    }
                    case val_tvar_: {
                        tmp = std::get<0>(exp);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = t%d", tnum, tmp);
                        tprintf2(buf, 6);
                        break;
                    }
                    case val_Tvar_: {
                        tmp = std::get<0>(exp);
                        // native var
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d", tnum, tmp);
                        tprintf2(buf, 6);
                        break;
                    }
                    case val_array_: {
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]", tnum, std::get<0>(exp), std::get<1>(exp));
                        tprintf2(buf, 13);
                        break;
                    }
                    default: symerror("array non const type error");
                }
                cerr << "property dims size" << endl;
                cerr << property.dims.size() << endl;
                int factor = 1;
                for(int j = i+1; j < s; j++){
                    factor *= property.dims[j];
                }
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d * %d", tnum, tnum, factor);
                tprintf2(buf, 2);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d + t%d", t_index_base, t_index_base, tnum);
                tprintf2(buf, 3);
            }
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d * 4", t_index_base, t_index_base);
            tprintf2(buf, 2);
            std::get<0>(ret) = index;
            std::get<1>(ret) = t_index_base;
            std::get<2>(ret) = val_array_;
        }
    }
    else {
        if(property.isConst){
            std::get<0>(ret) = std::get<0>(property.value[0]);
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_const_;
        }
        else {
            std::get<0>(ret) = index;
            std::get<1>(ret) = -1;
            std::get<2>(ret) = val_Tvar_;
        }
    }
    return ret;
}

void process_array(std::vector<retVal_t>& src, std::vector<retVal_t>& dst, std::string& pattern, std::vector<int>& dims){
    retVal_t zero;
    std::get<0>(zero) = 0;
    std::get<1>(zero) = -1;
    std::get<2>(zero) = val_const_;
    cerr << "Dimensions: " << dims.size() << endl;
    if(dims.size() == 1){
        for(int i = 0; i < src.size(); i++){
            dst.push_back(src[i]);
        }
        for(int i = 0; i < dims[0] - src.size(); i++){
            dst.push_back(zero);
        }
        return;
    }

    int factor = 1;
    for(int i = 1; i < dims.size(); i++){
        factor *= dims[i];
    }

    if(pattern.size()==2){
        // pattern: '{' '}'
        for(int j = 0; j < factor*dims[0]; j++){
            dst.push_back(zero);
        }
        return;
    }

    // pattern: '{' ... '}'
    int thisdimcnt = 0;
    int index = 0;
    int l = 1;
    int srcl = 0;
    int cnt = 0;
    while(true){
        index++;
        if(index == pattern.size()-1){
            if(thisdimcnt == dims[0]) break;
            std::vector<retVal_t> newSrc;
            std::vector<retVal_t> newDst;
            std::string newPattern(pattern, l, cnt);
            std::vector<int> newDims;
            for(int i = 0; i < cnt; i++){
                newSrc.push_back(src[srcl+i]);
            }
            newPattern = std::string("{") + newPattern;
            newPattern = newPattern + std::string("}");
            for(int i = 1; i < dims.size(); i++){
                newDims.push_back(dims[i]);
            }
            process_array(newSrc, newDst, newPattern, newDims);
            for (int i = 0; i < newDst.size(); i++) {
                dst.push_back(newDst[i]);
            }
            thisdimcnt++;
            for(int i = 0; i < (dims[0]-thisdimcnt)*factor; i++){
                dst.push_back(zero);
            }
            break;
        }
        else if(pattern[index] == 'n'){
            cnt++;
            if(cnt == factor){
                // end of a pattern
                std::vector<retVal_t> newSrc;
                std::vector<retVal_t> newDst;
                std::string newPattern(pattern, l, cnt);
                std::vector<int> newDims;
                newPattern = std::string("{") + newPattern + std::string("}");
                for(int i = 0; i < factor; i++){
                    newSrc.push_back(src[srcl+i]);
                }
                for(int i = 1; i < dims.size(); i++){
                    newDims.push_back(dims[i]);
                }
                process_array(newSrc, newDst, newPattern, newDims);
                for (int i = 0; i < newDst.size(); i++) {
                    dst.push_back(newDst[i]);
                }
                srcl += cnt;
                cnt = 0;
                l = index;
                thisdimcnt++;
            }
        }
        else if(pattern[index] == '{'){
            if(cnt != 0){
                std::vector<retVal_t> newSrc;
                std::vector<retVal_t> newDst;
                std::string newPattern(pattern, l, cnt);
                std::vector<int> newDims;
                for(int i = 0; i < cnt; i++){
                    newSrc.push_back(src[srcl+i]);
                }
                newPattern = "{" + newPattern + "}";
                for(int i = 1; i < dims.size(); i++){
                    newDims.push_back(dims[i]);
                }
                process_array(newSrc, newDst, newPattern, newDims);
                for (int i = 0; i < newDst.size(); i++) {
                    dst.push_back(newDst[i]);
                }
                thisdimcnt++;
                srcl+=cnt;
            }
            std::vector<retVal_t> newSrc;
            std::vector<retVal_t> newDst;
            std::string newPattern;
            std::vector<int> newDims;
            int arraycnt = 1;
            int ncnt = 0;
            while(arraycnt != 0){
                index++;
                if(pattern[index] == '{') arraycnt++;
                if(pattern[index] == '}') arraycnt--;
                if(pattern[index] == 'n') {
                    newSrc.push_back(src[srcl+ncnt]);
                    ncnt++;
                }
                newPattern.push_back(pattern[index]);
            }
            for(int i = 1; i < dims.size(); i++)newDims.push_back(dims[i]);
            newPattern = "{" + newPattern;
            process_array(newSrc, newDst, newPattern, newDims);
            for(int i = 0; i < newDst.size(); i++){
                dst.push_back(newDst[i]);
            }
            l = index + 1;
            srcl += ncnt;
            cnt = 0;
            thisdimcnt++;
        }
    }
}

void dispatchFuncDef(ASTPtr treeRoot) {
    auto node = (FuncDefAST*)treeRoot;
    if(print_flag4){
        std::string id = node->id;
        process_decl(id);
        process_op(id);
        return;
    }
    if(print_flag3){
        int paramnum = 0;
        if(node->ast_type()->withFuncFParam){
            auto paramchild = node->children[0];
            paramnum = getFuncParamNumber(paramchild);
        }
        else{
            paramnum = 0;
        }
        auto paramnumit = funcParamNum.find(node->id);
        if(paramnumit == funcParamNum.end()){
            funcParamNum.insert(std::make_pair(node->id, 0));
            paramnumit = funcParamNum.find(node->id);
        }
        paramnumit->second = paramnum;

        fprintf(yyout, "f_%s [%d]\n", node->id.c_str(), paramnum);
        if(node->id == "main"){
            std::string glo("global_func");
            auto it = funcOp.find(glo);
            if(it != funcOp.end()) {
                auto vecit = it->second.begin();
                while (vecit != it->second.end()) {
                    fprintf(yyout, "%s\n", vecit->c_str());
                    vecit++;
                }
            }
        }

        auto varit = funcVar.find(node->id);
        if(varit != funcVar.end()) {
            auto varvecit = varit->second.begin();
            char buf[100];
            while (varvecit != varit->second.end()) {
                switch (std::get<2>(*varvecit)) {
                    case val_tvar_: {
                        fprintf(yyout, "var t%d\n", std::get<0>(*varvecit));
                        break;
                    }
                    case val_Tvar_: {
                        fprintf(yyout, "var T%d\n", std::get<0>(*varvecit));
                        break;
                    }
                    case val_array_: {
                        fprintf(yyout, "var %d T%d\n", std::get<1>(*varvecit), std::get<0>(*varvecit));
                        break;
                    }
                    default:
                        symerror("var declaration type error");
                }
                varvecit++;
            }
        }

        auto it = funcOp.find(node->id);
        if(it != funcOp.end()) {
            auto vecit1 = it->second.begin();
            while (vecit1 != it->second.end()) {
                fprintf(yyout, "%s\n", vecit1->c_str());
                vecit1++;
            }
        }
        fprintf(yyout, "end f_%s\n", node->id.c_str());
        return;
    }

    cerr << "function name: " << node->id << endl;
    int lastNs = curNsNum;
    functabEntry entry;
    curNsNum = maxNsNum;
    nsRootTable.insert(std::make_pair(node, curNsNum));
    maxNsNum++;
    if(node->ast_type()->isReturnInt) entry.retInt = true;
    else entry.retInt = false;
    ASTPtr blockchild = nullptr;
    if(node->ast_type()->withFuncFParam){
        auto paramchild = node->children[0];
        blockchild = node->children[1];
        entry.paramNum = getFuncParamNumber(paramchild);
    }
    else{
        blockchild = node->children[0];
        entry.paramNum = 0;
    }
    funcTable.insert(std::make_pair(node->id, entry));

    char buf[100];
    std::string lastFunc;
    if(print_flag1) {
        lastFunc = curFunc;
        curFunc = node->id;
        std::vector<retVal_t> tmp;
        tmp.clear();
        funcVar.insert(std::make_pair(curFunc, tmp));
    }

    if(node->ast_type()->withFuncFParam){
        auto paramchild = node->children[0];
        dispatchFuncFParamList(paramchild);
    }

    // for main function, initialize variables
    if(print_flag1 && node->id == std::string("main")){
        auto it = nsRootTable.find(ASTRoot);
        if(it == nsRootTable.end()){
            symerror("global ns not defined");
        }
        int globalNsNum = it->second;
        auto varit = naVarTable.data.begin();
        while(varit != naVarTable.data.end()){
            auto key = varit->first;
            int varNsNum = key.second;
            int varIndex = varit->second;
            std::string varName = key.first;
            if(varNsNum == globalNsNum){
                auto varinfo = varTable.find(varIndex);
                auto initval = varinfo->second;
                if(initval.isInit){
                    if(initval.isArray){
                        int arraysize = initval.value.size();
                        for(int i = 0; i < arraysize; i++){
                            if(std::get<2>(initval.value[i]) == val_array_) {
                                int tnum = maxtCnt;
                                maxtCnt++;
                                dectvar(tnum);
                            }
                        }
                    }
                }
            }
            varit++;
        }
    }
    if(print_flag2 && node->id == std::string("main")){
        auto it = nsRootTable.find(ASTRoot);
        if(it == nsRootTable.end()){
            symerror("global ns not defined");
        }
        int globalNsNum = it->second;
        auto varit = naVarTable.data.begin();
        while(varit != naVarTable.data.end()){
            auto key = varit->first;
            int varNsNum = key.second;
            int varIndex = varit->second;
            std::string varName = key.first;
            if(varNsNum == globalNsNum){
                cerr << "varName: " << varName << endl;
                auto varinfo = varTable.find(varIndex);
                auto initval = varinfo->second;
                if(initval.isInit){
                    if(initval.isArray){
                        int arraysize = initval.value.size();
                        for(int i = 0; i < arraysize; i++){
                            cerr << "varName: " << varName << endl;
                            if(std::get<2>(initval.value[i]) == val_array_) {
                                int tnum = maxtCnt;
                                maxtCnt++;
                                dectvar(tnum);
                                char buf[100];

                                memset(buf, 0, sizeof(buf));
                                sprintf(buf, "t%d = %s", tnum, getInitValString(initval.value[i]).c_str());
                                tprintf2(buf, 13); // t%d = T%d[%d]

                                memset(buf, 0, sizeof(buf));
                                sprintf(buf, "T%d[%d] = t%d", varIndex, i * 4, tnum);
                                tprintf2(buf, 9);
                            }
                            else{
                                auto ansString = getInitValString(initval.value[i]);
                                char buf[100];
                                memset(buf, 0, sizeof(buf));
                                sprintf(buf, "T%d[%d] = %s", varIndex, i*4, ansString.c_str());
                                if(std::get<2>(initval.value[i]) == val_const_) {
                                    tprintf2(buf, 8); // T%d[%d] = %d
                                }
                                else {
                                    tprintf2(buf, 9); // T%d[%d] = T%d, T%d[%d] = t%d
                                }
                            }
                        }
                    }
                    else{
                        auto ansString = getInitValString(initval.value[0]);
                        char buf[100]; memset(buf, 0, sizeof(buf));
                        sprintf(buf, "T%d = %s", varIndex, ansString.c_str());
                        if(std::get<2>(initval.value[0]) == val_const_){
                            tprintf2(buf, 7);
                        }
                        else if(std::get<2>(initval.value[0]) == val_array_){
                            tprintf2(buf, 13);
                        }
                        else {
                            tprintf2(buf, 6);
                        }
                    }
                }
            }
            varit++;
        }
    }
    hasret = false;
    dispatchBlock(blockchild);
    if(!hasret){
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "return");
        tprintf2(buf, 22);
    }

    if(print_flag1) {
        curFunc = lastFunc;
    }
    curNsNum = lastNs;
}

int getFuncParamNumber(ASTPtr treeRoot) {
    auto node = (FuncParamAST*) treeRoot;
    int s = node->children.size();
    if(s == 1){
        return 1;
    }
    else if(s == 2){
        return getFuncParamNumber(node->children[0])+1;
    }
    else{
        symerror("getFuncParamNumber children number fault");
    }
}

int dispatchFuncFParamList(ASTPtr treeRoot) {
    // add FuncFParams into varTable
    auto node = (FuncParamAST*) treeRoot;
    int s = node->children.size();
    auto id = node->id;

    if(s == 1){
        dispatchFuncFParam(node->children[0], 0);
        return 1;
    }
    else if(s == 2){
        int tmp = dispatchFuncFParamList(node->children[0]);
        dispatchFuncFParam(node->children[1], tmp);
        return tmp+1;
    }
    else{
        symerror("FuncFParamList children number fault");
    }
}

void dispatchFuncFParam(ASTPtr treeRoot, int paramIndex){
    auto node = (FuncParamAST*)treeRoot;
    int s = node->children.size();
    auto id = node->id;
    if(s == 0){
        // add it into naVarTable, varTable
        auto key = std::make_pair(id, curNsNum);
        int index = naVarTable.insert(id, curNsNum);
        retVal_t val;
        std::get<0>(val) = index;
        std::get<1>(val) = -1;
        std::get<2>(val) = val_Tvar_;
        initValue initval;
        initval.isArray = false;
        initval.isInit = true;
        initval.isConst = false;
        initval.value.push_back(val);
        varTable.insert(std::make_pair(index, initval));

        char buf[100];
        retVal_t assignVal;
        std::get<0>(assignVal) = index;
        std::get<1>(assignVal) = -1;
        std::get<2>(assignVal) = val_Tvar_;
        assignVar(assignVal);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "T%d = p%d", index, paramIndex);
        tprintf2(buf, 23);
    }
    else if(s == 1){
        auto key = std::make_pair(id, curNsNum);
        int index = naVarTable.insert(id, curNsNum);
        retVal_t val;
        std::get<0>(val) = index;
        std::get<1>(val) = -1;
        std::get<2>(val) = val_array_;

        initValue initval;
        initval.isArray = true;
        initval.isInit = true;
        initval.isConst = false;
        initval.value.push_back(val);
        auto dims = dispatchExpList(node->children[0]);
        initval.dims.push_back(1);
        for (int i = 0; i < dims.size(); i++) initval.dims.push_back(std::get<0>(dims[i]));
        varTable.insert(std::make_pair(index, initval));

        char buf[100];
        retVal_t assignVal;
        std::get<0>(assignVal) = index;
        std::get<1>(assignVal) = -1;
        std::get<2>(assignVal) = val_Tvar_;
        assignVar(assignVal);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "T%d = p%d", index, paramIndex);
        tprintf2(buf, 23);
    }
}

std::vector<retVal_t> dispatchFuncRParamList(ASTPtr treeRoot) {
    auto node = (FuncParamAST*) treeRoot;
    int s = node->children.size();
    std::vector<retVal_t> ans;
    if(s == 1){
        auto exp = dispatchExp(node->children[0]);
        retVal_t val;
        int tnum = maxtCnt; maxtCnt++;
        std::get<0>(val) = tnum;
        std::get<1>(val) == -1;
        std::get<2>(val) = val_tvar_;
        dectvar(tnum);
        switchAndCopy(exp, tnum);
        ans.push_back(val);
        return ans;
    }
    else if(s == 2){
        auto child1 = dispatchFuncRParamList(node->children[0]);
        auto child2 = dispatchExp(node->children[1]);
        int childsize = child1.size();
        for(int i = 0; i < childsize; i++) ans.push_back(child1[i]);
        retVal_t val;
        int tnum = maxtCnt; maxtCnt++;
        std::get<0>(val) = tnum;
        std::get<1>(val) == -1;
        std::get<2>(val) = val_tvar_;
//        char buf[100]; memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum);
//        tprintf1(buf);
        dectvar(tnum);
        switchAndCopy(child2, tnum);
        ans.push_back(val);
        return ans;
    }
    else{
        symerror("dispatchFuncRParamList children number error");
    }
}

void dispatchBlock(ASTPtr treeRoot) {
    auto node = (BlockAST*)treeRoot;
    dispatchBlockItemList(node->children[0]);
}

void dispatchBlockItemList(ASTPtr treeRoot) {
    auto node = (BlockItemListAST*) treeRoot;
    int s = node->children.size();
    if(s == 0){
        return;
    }
    else if(s == 2){
        dispatchBlockItem(node->children[0]);
        dispatchBlockItemList(node->children[1]);
        return;
    }
    else{
        symerror("BlockItemList children number fault");
    }
}

void dispatchBlockItem(ASTPtr treeRoot) {
    auto node = (BlockItemAST*) treeRoot;
    int s = node->children.size();
    if(s == 1){
        auto child = node->children[0];
        if(child->ast_type()->type == TDecl){
            dispatchDecl(child);
        }
        else if(child->ast_type()->type == TStmt){
            dispatchStmt(child);
        }
        else{
            symerror("BlockItem children type error");
        }
    }
    else{
        symerror("BlockItem children number error");
    }
}

void dispatchStmt(ASTPtr treeRoot) {
    auto node = (StmtAST*) treeRoot;
    int lastNs = curNsNum;
    switch (node->type){
        case STIF: {
//            curNsNum = maxNsNum;
//            maxNsNum++;
            dispatchIfBlock(node->children[0]);
            break;
        }
        case STBLOCK: {
            curNsNum = maxNsNum;
            maxNsNum++;
            nsRootTable.insert(std::pair<ASTPtr , int>(node->children[0], curNsNum));
            dispatchBlock(node->children[0]);
            break;
        }
        case STWHILE: {
            cerr << "stmt while" << endl;
//            curNsNum = maxNsNum;
//            maxNsNum++;
            dispatchWhileBlock(node->children[0]);
            break;
        }
        case STASSIGN: {
            auto lvalchild = node->children[0];
            auto expchild = node->children[1];
            auto retval = dispatchLVal(lvalchild);
            auto retexp = dispatchExp(expchild);
            cerr << "ok" << endl;
            char buf[100];
            memset(buf, 0, sizeof(buf));
            int tnum = maxtCnt; maxtCnt++;
            dectvar(tnum);
            switchAndCopy(retexp, tnum);
            switch (std::get<2>(retval)) {
                case val_tvar_:
                    sprintf(buf, "t%d = t%d", std::get<0>(retval), tnum);
                    tprintf2(buf, 6);
                    break;
                case val_Tvar_:
                    sprintf(buf, "T%d = t%d", std::get<0>(retval), tnum);
                    tprintf2(buf, 6);
                    break;
                case val_array_: {
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "T%d[t%d] = t%d", std::get<0>(retval), std::get<1>(retval), tnum);
                    tprintf2(buf, 11);
                    break;
                }
                case val_const_:
                default:
                    symerror("stmt assign lval type fault");
            }
            memset(buf, 0, sizeof(buf));
            break;
        }
        case STSEMI: {
            break;
        }
        case STEXP: {
            bool lastfuncret = funcret;
            funcret = false;
            cerr << "stmt exp" << endl;
            auto childexp = node->children[0];
            dispatchExp(childexp);
            funcret = lastfuncret;
            break;
        }
        case STBREAK: {
            char buf[100];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "goto l%d", breakDst);
            tprintf2(buf, 15);
            break;
        }
        case STCONTINUE: {
            char buf[100];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "goto l%d", continueDst);
            tprintf2(buf, 15);
            break;
        }
        case STRETURN: {
            hasret = true;
            char buf[100];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "return");
            tprintf2(buf, 22);
            break;
        }
        case STRETURNEXP: {
            hasret = true;
            auto expchild = node->children[0];
            auto res = dispatchExp(expchild);
            char buf[100];
            memset(buf, 0, sizeof(buf));
            switch(std::get<2>(res)){
                case val_const_:
                    sprintf(buf, "return %d", std::get<0>(res));
                    tprintf2(buf, 20);
                    break;
                case val_tvar_:
                    sprintf(buf, "return t%d", std::get<0>(res));
                    tprintf2(buf, 21);
                    break;
                case val_Tvar_:
                    sprintf(buf, "return T%d", std::get<0>(res));
                    tprintf2(buf, 21);
                    break;
                case val_array_: {
                    cerr << "hit val_array" << endl;
                    int tnum = maxtCnt;
                    maxtCnt++;
                    dectvar(tnum);
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = T%d[t%d]", tnum, std::get<0>(res), std::get<1>(res));
                    tprintf2(buf, 13);
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "return t%d", tnum);
                    tprintf2(buf, 21);
                    break;
                }
                default:
                    symerror("return exp type error");
            }
            break;
        }
        default:
            symerror("Stmt type error");
    }
    curNsNum = lastNs;
}

void dispatchIfBlock(ASTPtr treeRoot) {
    auto node = (IfBlockAST*)treeRoot;
    int s = node->children.size();
    if(s == 3){
        auto cond = dispatchCond(node->children[0]);
        char buf[100];
        int elseLabel = maxlCnt;
        maxlCnt++;
        int nextLabel = maxlCnt;
        maxlCnt++;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", std::get<0>(cond), elseLabel);
        tprintf2(buf, 14);
        dispatchStmt(node->children[1]);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "goto l%d", nextLabel);
        tprintf2(buf, 15);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", elseLabel);
        tprintf2(buf, 16);
        dispatchStmt(node->children[2]);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "goto l%d", nextLabel);
        tprintf2(buf, 15);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", nextLabel);
        tprintf2(buf, 16);
    }
    else if(s == 2){
        auto cond = dispatchCond(node->children[0]);
        char buf[100];
        int nextLabel = maxlCnt;
        maxlCnt++;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", std::get<0>(cond), nextLabel);
        tprintf2(buf, 14);
        dispatchStmt(node->children[1]);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "goto l%d", nextLabel);
        tprintf2(buf, 15);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", nextLabel);
        tprintf2(buf, 16);
    }
    else{
        symerror("IfBlock children number error");
    }
}

void dispatchWhileBlock(ASTPtr treeRoot) {
    auto node = (WhileBlockAST*)treeRoot;
    int startLabel = maxlCnt;
    maxlCnt++;
    int nextLabel = maxlCnt;
    maxlCnt++;
    continueDst = startLabel;
    breakDst = nextLabel;
    char buf[100];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "l%d:", startLabel);
    tprintf2(buf, 16);
    auto cond = dispatchCond(node->children[0]);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "if t%d == 0 goto l%d", std::get<0>(cond), nextLabel);
    tprintf2(buf, 14);

    dispatchStmt(node->children[1]);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "goto l%d", startLabel);
    tprintf2(buf, 15);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "l%d:", nextLabel);
    tprintf2(buf, 16);
}

retVal_t dispatchLOrExp(ASTPtr treeRoot) {
    auto node = (LOrExp*) treeRoot;
    int s = node->children.size();
    if(s == 1){
        return dispatchLAndExp(node->children[0]);
    }
    else if(s == 2){
        int tnum = maxtCnt;
        maxtCnt++;
        char buf[100];

        dectvar(tnum);
        int label2 = maxlCnt;
        maxlCnt++;
        int nextlabel = maxlCnt;
        maxlCnt++;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = 0", tnum);
        tprintf2(buf, 7);
        auto orexp = dispatchLOrExp(node->children[0]);
        int num1 = std::get<0>(orexp);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", num1, label2);
        tprintf2(buf, 14);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = 1", tnum);
        tprintf2(buf, 7);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "goto l%d", nextlabel);
        tprintf2(buf, 15);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", label2);
        tprintf2(buf, 16);

        auto andexp = dispatchLAndExp(node->children[1]);
        int num2 = std::get<0>(andexp);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", num2, nextlabel);
        tprintf2(buf, 14);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = 1", tnum);
        tprintf2(buf, 7);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", nextlabel);
        tprintf2(buf, 16);

        retVal_t val;
        std::get<0>(val) = tnum;
        std::get<1>(val) = -1;
        std::get<2>(val) = val_tvar_;
        return val;
    }
    else{
        symerror("LOrExp children number fault");
    }
}

retVal_t dispatchLAndExp(ASTPtr treeRoot) {
    auto node = treeRoot;
    int s = node->children.size();
    if(s == 1){
        return dispatchEqExp(node->children[0]);
    }
    else if(s == 2){

        int tnum = maxtCnt;
        maxtCnt++;
        int falselabel = maxlCnt;
        maxlCnt++;
        int nextlabel = maxlCnt;
        maxlCnt++;
        char buf[100];
//        memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum);
//        tprintf1(buf);
        dectvar(tnum);

        auto andexp = dispatchLAndExp(node->children[0]);
        int num1 = std::get<0>(andexp);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", num1, falselabel);
        tprintf2(buf, 14);

        auto eqexp = dispatchEqExp(node->children[1]);
        int num2 = std::get<0>(eqexp);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "if t%d == 0 goto l%d", num2, falselabel);
        tprintf2(buf, 14);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = 1", tnum);
        tprintf2(buf, 7);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "goto l%d", nextlabel);
        tprintf2(buf, 15);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", falselabel);
        tprintf2(buf, 16);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = 0", tnum);
        tprintf2(buf, 7);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "l%d:", nextlabel);
        tprintf2(buf, 16);

        retVal_t val;
        std::get<0>(val) = tnum;
        std::get<1>(val) = -1;
        std::get<2>(val) = val_tvar_;
        return val;
    }
    else{
        symerror("LAndExp children number fault");
    }
}

retVal_t dispatchEqExp(ASTPtr treeRoot) {
    auto node = (ExpAST*) treeRoot;
    int s = node->children.size();
    retVal_t val;
    std::get<1>(val) = -1;
    std::get<2>(val) = val_tvar_;
    if(s == 1){
        auto exp = dispatchRelExp(node->children[0]);
        int tnum = maxtCnt;
        maxtCnt++;
        char buf[100];

        dectvar(tnum);
        switchAndCopy(exp, tnum);
        std::get<0>(val) = tnum;
        return val;
    }
    else if(s == 3){
        int tnum = maxtCnt; maxtCnt++;
        int tnum1 = maxtCnt; maxtCnt++;
        int tnum2 = maxtCnt; maxtCnt++;
        char buf[100];

        dectvar(tnum);

        dectvar(tnum1);

        dectvar(tnum2);
        auto eqexp = dispatchEqExp(node->children[0]);
        auto relexp = dispatchRelExp(node->children[2]);
        switchAndCopy(eqexp, tnum1);
        switchAndCopy(relexp, tnum2);
        auto op = ((OpAST*)(node->children[1]))->op;
        if(op == BOP_EQ){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d == t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else if(op == BOP_NE){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d != t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else{
            symerror("Op type error");
        }
        std::get<0>(val) = tnum;
        return val;
    }
    else{
        symerror("EqExp children number error");
    }
}

retVal_t dispatchRelExp(ASTPtr treeRoot) {
    auto node = (ExpAST*) treeRoot;
    int s = node->children.size();
    cerr << "dispatchRelExp: " << s << endl;

    retVal_t val;
    std::get<1>(val) = -1;
    std::get<2>(val) = val_tvar_;
    if(s == 1){
        auto exp = dispatchAddExp(node->children[0]);
        int tnum = maxtCnt;
        maxtCnt++;
        char buf[100];
//        memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum);
//        tprintf1(buf);
        dectvar(tnum);
        switchAndCopy(exp, tnum);
        std::get<0>(val) = tnum;
        return val;
    }
    else if(s == 3){
        int tnum = maxtCnt; maxtCnt++;
        int tnum1 = maxtCnt; maxtCnt++;
        int tnum2 = maxtCnt; maxtCnt++;
        char buf[100];
//        memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum);
//        tprintf1(buf);
        dectvar(tnum);
//        memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum1);
//        tprintf1(buf);
        dectvar(tnum1);
//        memset(buf, 0, sizeof(buf));
//        sprintf(buf, "var t%d", tnum2);
//        tprintf1(buf);
        dectvar(tnum2);
        auto relexp = dispatchRelExp(node->children[0]);
        auto addexp = dispatchAddExp(node->children[2]);
        switchAndCopy(relexp, tnum1);
        switchAndCopy(addexp, tnum2);

        auto op = ((OpAST*)(node->children[1]))->op;
        if(op == '<'){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d < t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else if(op == '>'){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d > t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else if(op == BOP_LE){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d <= t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else if(op == BOP_GE){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = t%d >= t%d", tnum, tnum1, tnum2);
            tprintf2(buf, 3);
        }
        else{
            symerror("Op type error");
        }
        std::get<0>(val) = tnum;
        return val;
    }
    else{
        symerror("EqExp children number error");
    }
}

retVal_t dispatchCond(ASTPtr treeRoot) {
    auto node = (CondAST*) treeRoot;
    auto ret = dispatchLOrExp(node->children[0]);
    return ret;
}

ASTPtr ASTRoot = nullptr;
unsigned NestListAST::addMember(ASTPtr p) {
    value.push_back(p);
    return static_cast<unsigned int>(value.size());
}
void asterr(const char* s){
    fprintf(stderr, "ERROR in the AST!, %s\n", s);
    exit(1);
    abort();
}
