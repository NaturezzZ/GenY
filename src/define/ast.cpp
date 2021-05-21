//
// Created by Naiqian on 2021/4/25.
//
#include "define/ast.h"
#include "define/symtab.h"
void attachNode(ASTPtr father, ASTPtr child){
    child->father = father;
    auto it = father->children.begin();
    while(it != father->children.end()){
        (*it)->siblings.push_back(child);
        child->siblings.push_back(*it);
        it++;
    }
    father->children.push_back(child);
    // }
}

void dispatchRoot(ASTPtr treeRoot){
    // create a global NS and step into
    std::map<ASTPtr, int>::iterator it;
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
            auto it = funcTable.find(id);
            if(it != funcTable.end()) { symerror("the function is redefined"); }
            else{
                // not redefined, add it into function table
                funcTable.insert(std::pair<std::string, functabEntry>(id, functabEntry(p->pNum)));
            }
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
            //check whether function is redefined
            auto it = funcTable.find(id);
            if(it != funcTable.end()) { symerror("the function is redefined"); }
            else{
                // not redefined, add it into function table
                funcTable.insert(std::pair<std::string, functabEntry>(id, functabEntry(p->pNum)));
            }
        }
    }
}

void dispatchDecl(ASTPtr treeRoot) {
    auto node = (DeclAST*) treeRoot;
    auto child = treeRoot->children[0];
    if(node->ast_type()->isConstDecl) {
        // Decl -> ConstDecl
        dispatchDefList(child, node->decls);
    }
    else {
        // Decl -> VarDecl
        dispatchDefList(child, node->decls);
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
    if(childsize == 1){
        // ConstDef -> IDENT '=' ConstInitVal
        val.isConst = true;
        val.isArray = false;
        val.isInit = true;
        auto initlist = dispatchConstInitList(node->children[0]);
    }
    else if(childsize == 2){
        // ConstDef -> IDENT Dimensions_list '=' ConstInitVal
        val.isConst = true;
        val.isArray = true;
        val.isInit = true;
        // TODO: init value
    }
    else{
        symerror("dispatchConstDef children size error");
    }
}

void dispatchDefList(ASTPtr treeRoot, std::map<std::string, initValue>& target) {
    auto node = (DefListAST*) treeRoot;
    if(node->ast_type()->isDest){
        // DefList -> Def
        auto child = node->children[0];
        dispatchDef(child, target);
    }
    else{
        // DefList -> Deflist ',' Def
        auto childList = node->children[0];
        auto childDef = node->children[1];
        dispatchDefList(childList, target);
        dispatchDef(childDef, target);
    }
}

void dispatchDef(ASTPtr treeRoot, std::map<std::string, initValue> & target) {
    // this function add definition and its initial value into target
    auto node = (DefAST*)treeRoot;
    initValue val;
    if(node->isArray){
        //is array
        auto dim_p = treeRoot->children[0];
        val.isArray = true;
        dispatchDimensionsList(dim_p, node->dimensions);
        if(node->withInitVal){
            val.isInit = true;
            auto init_p = treeRoot->children[1];
            dispatchInitList(init_p, val.value);
        }
        else {
            val.isInit = false;
        }
    }
    else{
        //not array
        val.isArray = false;
        if(node->withInitVal){
            val.isInit = true;
            auto init_p = treeRoot->children[0];
            dispatchInitList(init_p, val.value);
        }
        else{
            val.isInit = false;
        }
    }
    // above put initial value into val
    // below put val and id into target
    if(node->isConst){
        val.isConst = true;
    }
    val.dims.clear();
    for(int dimension : node->dimensions) val.dims.push_back(dimension);

    target.insert(std::pair<std::string, initValue>(node->id, val));
    int index = naVarTable.insert(node->id, curNsNum);
    varTable[index] = val; // may be incorrect
    // add def into naVarTable and varTable
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
        auto it = constexplist.begin();
        while(it != constexplist.end()){
            res.push_back(*it);
            it++;
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
        return dispatchInitList(node->children[0], target)+
               dispatchInitVal(node->children[1], target);
    }
    else{
        symerror("In dispatchInitList, children size fault");
    }
}

std::string dispatchConstInitVal(ASTPtr treeRoot, std::vector<retVal_t>& target){
    auto node = (NestListAST*)treeRoot;
    int s = node->children.size();
    if(s == 0){
        return std::string("{}");
    }
    else if(s == 1){
        auto child = node->children[0];
        if(child->ast_type()->type == TConstExp){
            auto val = dispatchConstExp(child);
            if(std::get<2>(val) != val_const_) symerror("In dispatchConstInitVal, ConstExp is not const");
            target.push_back(val);
            return std::string("n");
        }
        else{
            return std::string("{") + dispatchConstInitVal(child, target) + std::string("}");
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
        return dispatchConstInitList(node->children[0], target) +
               dispatchConstInitVal(node->children[1], target);
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
        auto add = dispatchAddExp(node->children[2]);
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
                    tprintf2(buf);
                    break;
                case '-':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d - t%d", restnum, tnum1, tnum2);
                    tprintf2(buf);
                    break;
                default:
                    symerror("unrecognized add operator in dispatchAddExp");
            }
            return ret;
        }
    }
}

retVal_t dispatchMulExp(ASTPtr treeRoot) {
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
//        auto Tptr1 = (ExpAST *) ptr1;
//        auto Tptr3 = (ExpAST *) ptr3;
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
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "var t%d\n", tnum1);
            tprintf1(buf);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "var t%d\n", tnum2);
            tprintf1(buf);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "var t%d\n", tnum3);
            tprintf1(buf);
            switchAndCopy(mul, tnum1);
            switchAndCopy(unary, tnum2);
            switch (op) {
                case '*':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d * t%d\n", tnum3, tnum1, tnum2);
                    tprintf2(buf);
                    break;
                case '/':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d / t%d\n", tnum3, tnum1, tnum2);
                    tprintf2(buf);
                    break;
                case '%':
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "t%d = t%d % t%d\n", tnum3, tnum1, tnum2);
                    tprintf2(buf);
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
    auto node = (ExpAST*) treeRoot;
    int size = static_cast<int>(node->children.size());
    if(size == 0){
        // UnaryExp -> Ident '(' ')'
        char buf[100];
        int tnum = maxtCnt;
        maxtCnt++;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "var t%d\n", tnum);
        tprintf1(buf);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "t%d = call f_%s\n", tnum, node->id.c_str());
        retVal_t ret;
        std::get<2>(ret) = val_tvar_;
        std::get<1>(ret) = -1;
        std::get<0>(ret) = tnum;
        return ret;
    }
    auto child0 = node->children[0];
    if(size == 1 && child0->ast_type()->type == TExp){
        // UnaryExp -> Primary
        auto ptr1 = (ExpAST*) (node->children[0]);
        auto primary = dispatchPrimaryExp(ptr1);
        return primary;
    }
    else if(size == 1 && child0->ast_type()->type == TFuncParam){
        // UnaryExp -> IDENT '(' FuncRParams ')'
        // TODO: FuncRParams
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
                return unary;
            }
            case val_array_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "var t%d\n", tnum);
                tprintf1(buf);
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]\n", tnum, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf);
                        break;
                    case '-':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = -T%d[t%d]\n", tnum, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf);
                        break;
                    case '!':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = !T%d[t%d]\n", tnum, std::get<0>(unary), std::get<1>(unary));
                        tprintf2(buf);
                        break;
                }
                std::get<0>(unary) = tnum;
                return unary;
            }
            case val_tvar_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "var t%d\n", tnum);
                tprintf1(buf);
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = t%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                    case '-':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = -t%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                    case '!':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = !t%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                }
                std::get<0>(unary) = tnum;
                return unary;
            }
            case val_Tvar_: {
                int tnum = maxtCnt;
                maxtCnt++;
                char buf[100];
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "var t%d\n", tnum);
                tprintf1(buf);
                switch (ptr1->op) {
                    case '+':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                    case '-':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = -T%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                    case '!':
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = !T%d\n", tnum, std::get<0>(unary));
                        tprintf2(buf);
                        break;
                }
                std::get<0>(unary) = tnum;
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
    switch(child->ast_type()->type){
        case TExp: {
            auto exp = dispatchExp(child);
            return exp;
        }
        case TLVal: {
            auto lval = dispatchLVal(child);
            return lval;
        }
        case TNumber:{
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
    int s = node->children.size();
    if(s == 0){
        return res;
    }
    auto exp = dispatchExp(node->children[0]);
    auto explist = dispatchExpList(node->children[1]);
    res.push_back(exp);
    auto it = explist.begin();
    while(it!=explist.end()){
        res.push_back(*it);
        it++;
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
        std::vector<retVal_t> explist(dispatchExpList(node->children[1]));
        int s = static_cast<int>(explist.size());
        if(s == 0)symerror("const array explist size = 0");

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
                std::get<0>(ret) = property.value[cnt];
                std::get<1>(ret) = -1;
                std::get<2>(ret) = val_const_;
            }
            else {
                int tempconst = maxtCnt;
                maxtCnt++;
                char buf[100];
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "var t%d\n", tempconst);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = %d\n", tempconst, cnt);
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
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "var t%d\n", t_index_base);
            tprintf1(buf);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "t%d = 0\n", t_index_base);
            tprintf2(buf);

            for(int i = 0; i < s; i++){
                auto exp = explist[i];
                int tnum = maxtCnt;
                maxtCnt++;
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "var t%d\n", tnum);
                tprintf1(buf);
                int tmp = 0;
                switch (std::get<2>(exp)){
                    case val_const_:
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = %d\n", tnum, std::get<0>(exp));
                        tprintf2(buf);
                        break;
                    case val_tvar_:
                        tmp = std::get<0>(exp);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = t%d\n", tnum, tmp);
                        tprintf2(buf);
                        break;
                    case val_Tvar_:
                        tmp = std::get<0>(exp);
                        // native var
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d\n", tnum, tmp);
                        tprintf2(buf);
                        break;
                    case val_array_:
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "t%d = T%d[t%d]\n", tnum, std::get<0>(exp), std::get<1>(exp));
                        tprintf2(buf);
                        break;
                    default: symerror("array non const type error");
                }
                int factor = 1;
                for(int j = i+1; j < s; j++){
                    factor *= property.dims[j];
                }
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d * %d\n", tnum, tnum, factor);
                tprintf2(buf);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "t%d = t%d + t%d\n", t_index_base, t_index_base, tnum);
                tprintf2(buf);
            }
            std::get<0>(ret) = index;
            std::get<1>(ret) = t_index_base;
            std::get<2>(ret) = val_array_;
        }
    }
    else {
        if(property.isConst){
            std::get<0>(ret) = property.value[0];
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

void dispatchFuncDef(ASTPtr treeRoot) {}
void dispatchFuncParam(ASTPtr treeRoot) {}
void dispatchBlock(ASTPtr treeRoot) {}
void dispatchBlockItemList(ASTPtr treeRoot) {}
void dispatchBlockItem(ASTPtr treeRoot) {}
void dispatchStmt(ASTPtr treeRoot) {}
void dispatchIfBlock(ASTPtr treeRoot) {}
void dispatchWhileBlock(ASTPtr treeRoot) {}

void dispatchLOrExp(ASTPtr treeRoot) {}
void dispatchCond(ASTPtr treeRoot) {}


void process_array(std::vector<retVal_t> src, std::vector<retVal_t> dst, std::string pattern);

ASTPtr ASTRoot = nullptr;
unsigned NestListAST::addMember(ASTPtr p) {
    value.push_back(p);
    return static_cast<unsigned int>(value.size());
}
void asterr(const char* s){
    fprintf(stderr, "ERROR in the AST!, %s\n", s);
    abort();
}