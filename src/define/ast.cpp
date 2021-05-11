//
// Created by Naiqian on 2021/4/25.
//
#include "define/ast.h"
void attachNode(ASTPtr father, ASTPtr child){
    child->father = father;
    // if(father->children.size() == 0){
    //     father->children.push_back(child);

    // }
    // else{
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
    
}

void dispatchCompUnit(ASTPtr treeRoot){

}



ASTPtr ASTRoot = nullptr;
unsigned NestListAST::addMember(ASTPtr p) {
    value.push_back(p);
    return value.size();
}
void asterr(const char* s){
    fprintf(stderr, "ERROR in the AST!, %s\n", s);
    abort();
}