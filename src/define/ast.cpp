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
//            auto p = (DeclAST*)(node->children[1]);
//            auto it = (p->decls).begin();
//            while(it != (p->decls).end()){
//                naVarTable.insert(it->first, curNsNum);
//                it++;
//            }
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
//            auto p = (DeclAST*)(node->children[0]);
//            auto it = (p->decls).begin();
//            while(it != (p->decls).end()){
//                naVarTable.insert(it->first, curNsNum);
//                it++;
//            }
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

void dispatchDimensionsList(ASTPtr treeRoot, std::vector<int> & target){
    auto node = (DimensionsListAST*)treeRoot;
    int childnum = static_cast<int>(node->children.size());
    if(childnum == 1){
        // leaf
        dispatchConstExp(node->children[0], 1);
        auto constExpValue = (ConstExpAST*) (node->children[0]);
        target.push_back(constExpValue->value);
    }
    else if(childnum == 2){
        // not leaf
        dispatchDimensionsList(node->children[0], target);
        dispatchConstExp(node->children[1], true);
        auto constExpValue = (ConstExpAST*) (node->children[1]);
        target.push_back(constExpValue->value);
    }
    else {
        symerror("DimensionsList children number error");
    }
}
void dispatchInitList(ASTPtr treeRoot, std::vector<int> & target) {
    // TODO: finish Initial list (multi-dimensional array)
}
void dispatchConstExp(ASTPtr treeRoot, bool isConst){
    if(isConst) {
        auto node = (ExpAST *) treeRoot;
        dispatchAddExp(node->children[0], true);
        node->value = ((ExpAST*)(node->children[0]))->value;
    }
    else {
        // TODO: not const, need temporary var
    }
}
void dispatchNestList(ASTPtr treeRoot) {}
void dispatchAddExp(ASTPtr treeRoot, bool isConst) {
    if(isConst){
        auto node = (ExpAST*) treeRoot;
        int size = static_cast<int>(node->children.size());
        if(size == 1){
            // MulExp
            dispatchMulExp(node->children[0], isConst);
            node->value = ((ExpAST*)(node->children[0]))->value;
        }
        else if(size == 3){
            // AddExp Plus_Minus MulExp
            dispatchAddExp(node->children[0], isConst);
            dispatchMulExp(node->children[2], isConst);
            auto op = (OpAST*)(node->children[1]);
            if(op->op == '+') {
                node->value = ((ExpAST*)(node->children[0]))->value;
                node->value += ((ExpAST*)(node->children[2]))->value;
            }
            else if(op->op == '-'){
                node->value = ((ExpAST*)(node->children[0]))->value;
                node->value -= ((ExpAST*)(node->children[2]))->value;
            }
            else {
                symerror(R"(op is not '+' or '-' when dispatching addExp)");
            }
        }
    }
    else {
        // TODO: AddExp not const
    }
}
void dispatchMulExp(ASTPtr treeRoot, bool isConst) {
    auto node = (ExpAST*) treeRoot;
    if(isConst){
        int size = static_cast<int>(node->children.size());
        if(size == 1){
            dispatchUnaryExp(node->children[0], isConst);
            auto tmp = (ExpAST*)(node->children[0]);
            node->value = tmp->value;
        }
        else if(size == 3){
            auto ptr1 = node->children[0];
            auto ptr3 = node->children[2];
            auto Tptr1 = (ExpAST*)ptr1;
            auto Tptr3 = (ExpAST*)ptr3;
            dispatchMulExp(ptr1, isConst);
            dispatchUnaryExp(ptr3, isConst);
            auto op = ((OpAST*)(node->children[1]))->op;
            switch(op){
                case '*':
                    node->value = (Tptr1->value) * (Tptr3->value);
                    break;
                case '/':
                    node->value = (Tptr1->value) / (Tptr3->value);
                    break;
                case '%':
                    node->value = (Tptr1->value) % (Tptr3->value);
                    break;
                default: symerror("no correct op in dispatchMulExp");
            }
        }
        else{
            symerror("dispatch MulExp error");
        }
    }
    else {
        // TODO: not const MulExp
    }
}
void dispatchUnaryExp(ASTPtr treeRoot, bool isConst) {
    auto node = (ExpAST*) treeRoot;
    if(isConst){
        int size = static_cast<int>(node->children.size());
        if(size == 1){
            // UnaryExp -> Primary
            auto ptr1 = (ExpAST*) (node->children[0]);
            dispatchPrimaryExp(ptr1, isConst);
            node->value = ptr1->value;
        }
        else if(size == 2){
            // UnaryExp -> UnaryOp UnaryExp
            auto ptr1 = (OpAST*) (node->children[0]);
            auto ptr2 = (ExpAST*) (node->children[1]);
            dispatchUnaryExp(ptr2, isConst);
            auto op = ptr1->op;
            if(op == '+'){
                node->value = ptr2->value;
            }
            else if(op == '-'){
                node -> value = -(ptr2->value);
            }
            else if(op == '!'){
                node->value = !(ptr2->value);
            }
            else{
                symerror("UnaryOp not supported");
            }
        }
        else{
            symerror("dispatch UnaryExp err");
        }
    }
    else{
        // TODO: not const Unary
    }
}
void dispatchPrimaryExp(ASTPtr treeRoot, bool isConst) {
    auto node = (PrimaryExpAST*)treeRoot;
    auto child = node->children[0];
    if(isConst) {
        switch (node->type) {
            case 0:
                dispatchExp(child, isConst);
                node->value = ((ExpAST *) child)->value;
                break;
            case 1:
                dispatchLVal(child, isConst);
                node->value = ((ExpAST *) child)->value;
                break;
            case 2:
                node->value = ((NumberAST *) child)->value;
                break;
            default:
                symerror("primary exp not hit");
        }
    }
    else {
        //TODO: non const primary exp
    }
}
void dispatchFuncDef(ASTPtr treeRoot) {}
void dispatchFuncParam(ASTPtr treeRoot) {}
void dispatchExpList(ASTPtr treeRoot) {

}
void dispatchBlock(ASTPtr treeRoot) {}
void dispatchBlockItemList(ASTPtr treeRoot) {}
void dispatchBlockItem(ASTPtr treeRoot) {}
void dispatchStmt(ASTPtr treeRoot) {}
void dispatchIfBlock(ASTPtr treeRoot) {}
void dispatchWhileBlock(ASTPtr treeRoot) {}
void dispatchExp(ASTPtr treeRoot, bool isConst) {
    auto node = (ExpAST*) treeRoot;
    if(isConst){
        dispatchAddExp(node->children[0], isConst);
        node->value = ((ExpAST*)(node->children[0]))->value;
    }
    else{
        // TODO: non const
    }
}


void dispatchLOrExp(ASTPtr treeRoot) {}
void dispatchCond(ASTPtr treeRoot) {}
void dispatchLVal(ASTPtr treeRoot, bool isConst) {
    auto node = (LValAST*) treeRoot;
    auto child = (ExpListAST*) (node->children[0]);
    if(isConst){
        int size = static_cast<int>(node->children.size());
        auto key = std::pair(node->id, curNsNum);
        auto index = naVarTable.data[key];
        auto tmpVal = varTable[index].value;
        auto tmpDim = varTable[index].dims;
        if(child->ast_type()->isEmpty){
            // ExpList is empty
            node->value = tmpVal[0];
        }
        else{
            // ExpList not empty
            std::vector<int> tmp;
            dispatchDimensionsList(child, tmp);
            int cnt = 0;
            int s = static_cast<int>(tmp.size());
            for(int i = 0; i<s; i++){
                int fac = 1;
                for(int j = i+1; j <= s-1; j++) fac *= tmpDim[j];
                cnt += (fac*tmp[i]);
            }
            node->value = tmpVal[cnt];
        }
    }
    else{
        // TODO: not const
    }
}

void dispatchNumber(ASTPtr treeRoot) {
    // do nothing
}
void dispatchOp(ASTPtr treeRoot) {
    // do nothing
}

ASTPtr ASTRoot = nullptr;
unsigned NestListAST::addMember(ASTPtr p) {
    value.push_back(p);
    return static_cast<unsigned int>(value.size());
}
void asterr(const char* s){
    fprintf(stderr, "ERROR in the AST!, %s\n", s);
    abort();
}