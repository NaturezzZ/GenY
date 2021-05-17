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

void scanTree(ASTPtr treeRoot){
    auto type = treeRoot->ast_type_.get()->type;
    switch(type){
        case TRoot:
            dispatchRoot(treeRoot);
            break;
        case TCompUnit:
            dispatchCompUnit(treeRoot);
            break;
        case TDecl:
            dispatchDecl(treeRoot);
            break;
        case TDefList:
            dispatchDefList(treeRoot);
            break;
        case TDef:
            dispatchDef(treeRoot);
            break;
        case TInitList:
            dispatchInitList(treeRoot);
            break;
        case TConstExp:
            dispatchConstExp(treeRoot);
            break;
        case TNestList:
            dispatchNestList(treeRoot);
            break;
        case TAddExp:
            dispatchAddExp(treeRoot);
            break;
        case TFuncDef:
            dispatchFuncDef(treeRoot);
            break;
        case TFuncParam:
            dispatchFuncParam(treeRoot);
            break;
        case TExpList:
            dispatchExpList(treeRoot);
            break;
        case TBlock:
            dispatchBlock(treeRoot);
            break;
        case TBlockItemList:
            dispatchBlockItemList(treeRoot);
            break;
        case TBlockItem:
            dispatchBlockItem(treeRoot);
            break;
        case TStmt:
            dispatchStmt(treeRoot);
            break;
        case TIfBlock:
            dispatchIfBlock(treeRoot);
            break;
        case TWhileBlock:
            dispatchWhileBlock(treeRoot);
            break;
        case TExp:
            dispatchExp(treeRoot);
            break;
        case TLOrExp:
            dispatchLOrExp(treeRoot);
            break;
        case TCond:
            dispatchCond(treeRoot);
            break;
        case TLVal:
            dispatchLVal(treeRoot);
            break;
        case TNumber:
            dispatchNumber(treeRoot);
            break;
        case TOp:
            dispatchOp(treeRoot);
            break;
        default:
            asterr("Fall into unrecognized AST type.");
    }
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
        nsRootTable.insert(treeRoot, curNsNum);
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
            auto p = (DeclAST*)(node->children[1]);
            auto it = (p->decls).begin();
            while(it != (p->decls).end()){
                naVarTable.insert(it->first, curNsNum);
                it++;
            }
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
            auto p = (DeclAST*)(node->children[0]);
            auto it = (p->decls).begin();
            while(it != (p->decls).end()){
                naVarTable.insert(it->first, curNsNum);
                it++;
            }
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
    if(node->ast_type()->isConstDecl){
        // Decl -> ConstDecl
        dispatchDefList(child, node->decls);
    }
    else{
        // Decl -> VarDecl
        dispatchDefList(child, node->decls);
    }
}
void dispatchDefList(ASTPtr treeRoot, std::map<std::string, int>& target) {
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
void dispatchDef(ASTPtr treeRoot, std::map<std::string, int> & target) {
    auto node = (DefAST*)treeRoot;

}
void dispatchInitList(ASTPtr treeRoot) {}
void dispatchConstExp(ASTPtr treeRoot){}
void dispatchNestList(ASTPtr treeRoot) {}
void dispatchAddExp(ASTPtr treeRoot) {}
void dispatchFuncDef(ASTPtr treeRoot) {}
void dispatchFuncParam(ASTPtr treeRoot) {}
void dispatchExpList(ASTPtr treeRoot) {}
void dispatchBlock(ASTPtr treeRoot) {}
void dispatchBlockItemList(ASTPtr treeRoot) {}
void dispatchBlockItem(ASTPtr treeRoot) {}
void dispatchStmt(ASTPtr treeRoot) {}
void dispatchIfBlock(ASTPtr treeRoot) {}
void dispatchWhileBlock(ASTPtr treeRoot) {}
void dispatchExp(ASTPtr treeRoot) {}
void dispatchLOrExp(ASTPtr treeRoot) {}
void dispatchCond(ASTPtr treeRoot) {}
void dispatchLVal(ASTPtr treeRoot) {}
void dispatchNumber(ASTPtr treeRoot) {}
void dispatchOp(ASTPtr treeRoot) {}

ASTPtr ASTRoot = nullptr;
unsigned NestListAST::addMember(ASTPtr p) {
    value.push_back(p);
    return value.size();
}
void asterr(const char* s){
    fprintf(stderr, "ERROR in the AST!, %s\n", s);
    abort();
}