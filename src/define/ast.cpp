//
// Created by Naiqian on 2021/4/25.
//
#include "define/ast.h"
void attachNode(ASTPtr father, ASTPtr child){
    child->father = father;
    auto it = father->children.begin();
    while(it++ != father->children.end()){
        (*it)->siblings.push_back(child);
        child->siblings.push_back(*it);
    }
    father->children.push_back(child);
}
ASTPtr ASTRoot = nullptr;
