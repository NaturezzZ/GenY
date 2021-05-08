//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_AST_H
#define GENY_AST_H
#include "define/type.h"
#include <bits/stdc++.h>

#include <utility>

class BaseAST;
class DefAST;

using ASTPtr = BaseAST* ;
using ASTPtrList = std::vector<ASTPtr>;

class BaseAST {
public:
    virtual ~BaseAST() = default;
//    // return true if current AST is a literal value
//    virtual bool IsLiteral() const = 0;
//    // return true if current AST is a initializer list
//    virtual bool IsInitList() const = 0;

    // dump the content of AST (XML format) to output stream
//    virtual void Dump(std::ostream &os) const = 0;
    const TypePtr &set_ast_type(const TypePtr &ast_type) { return ast_type_ = ast_type; }
    // getters
    TypePtr & ast_type() { return ast_type_; }

    ASTPtr father;
    ASTPtrList children;
    ASTPtrList siblings;
    BaseAST(const BaseAST& obj){
        father = obj.father;
        children = std::vector<ASTPtr>(children);
        siblings = std::vector<ASTPtr>(siblings);
        ast_type_ = obj.ast_type_;
    }
    BaseAST(){
        father = this;
    }
    virtual std::vector<int> & getValueVectorInt() = 0;
    virtual int & getValueInt() = 0;
//private:
    TypePtr ast_type_;
};

class RootAST : public BaseAST {
public:
    RootAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TRoot;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class CompUnitAST : public BaseAST {
public:
    CompUnitAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TCompUnit;
        tmpType.get()->withCompUnit = false;
        tmpType.get()->withDecl = false;
        tmpType.get()->withFuncDef = false;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

// (const) variable declaration
class DeclAST : public BaseAST {
public:
    DeclAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDecl;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class DefListAST : public BaseAST{
public:
    std::vector<DefAST> members;
    uint32_t addMember(DefAST * def){
        members.push_back((*def));
        return members.size();
    }
    explicit DefListAST(const DefListAST* obj):BaseAST(*obj){
        members = std::vector<DefAST>(obj->members);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDefList;
        set_ast_type(tmpType);
    }
    DefListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDefList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

// (const) variable definition
class DefAST : public BaseAST {
public:
    std::string id;
    std::vector<int> dimensions;
    std::vector<int> initval;

    DefAST(const DefAST &obj):BaseAST(obj) {
        id = std::string(obj.id);
        dimensions = std::vector<int>(obj.dimensions);
        initval = std::vector<int>(initval);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
    }

    DefAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
    }

    DefAST(const std::string & id_, const std::vector<int> & dimensions_, const std::vector<int> & initval_):BaseAST(){
        id = std::string(id_);
        dimensions = std::vector<int>(dimensions_);
        initval = std::vector<int>(initval_);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
    }

    DefAST(const std::string & id_, const std::vector<int> & dimensions_):BaseAST(){
        id = std::string(id_);
        dimensions = std::vector<int>(dimensions_);
        initval = std::vector<int>(dimensions.size());
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

// initializer list (for array initialization)
class InitListAST : public BaseAST {
public:
    std::vector<int> value;
    unsigned addMember(int ipt){
        value.push_back(ipt);
        return value.size();
    }
    unsigned addMember(const std::vector<int> & ipt){
        int s = ipt.size();
        for(int i = 0; i < s; i++) value.push_back(ipt[i]);
        return value.size();
    }
    InitListAST(const InitListAST & obj):BaseAST(obj){
        value = std::vector<int>(obj.value);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TInitList;
        set_ast_type(tmpType);
    }

    InitListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TInitList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        return value;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class DimensionsListAST : public BaseAST {
public:
    std::vector<int> value;
    unsigned addMember(int dim){
        value.push_back(dim);
        return value.size();
    }
    unsigned addMember(const std::vector<int> & obj){
        int s = obj.size();
        for(int i = 0; i < s; i++) value.push_back(obj[i]);
        return value.size();
    }
    DimensionsListAST(const DimensionsListAST & obj):BaseAST(obj){
        value = std::vector<int>(obj.value);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TInitList;
        set_ast_type(tmpType);
    }
    DimensionsListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TInitList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        return value;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class ConstExpAST : public BaseAST {
public:
    int value;
    ConstExpAST(){
        value = 0;
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TConstExp;
        set_ast_type(tmpType);
    }
    int & getValueInt() override{
        return value;
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
};

class NestListAST : public BaseAST {
public:
    ASTPtrList value;
    unsigned addMember(ASTPtr p){
        value.push_back(p);
        return value.size();
    }
    NestListAST(const NestListAST & obj):BaseAST(obj){
        value = ASTPtrList(obj.value);
    }
    NestListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TNestList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class AddExpAST : public BaseAST {
public:
    AddExpAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TAddExp;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

// function declaration
class FuncDeclAST : public BaseAST {
public:

};
// function definition
class FuncDefAST : public BaseAST {
public:
    std::string id;
    FuncDefAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncDef;
        set_ast_type(tmpType);
    }

    explicit FuncDefAST(const std::string & id_):BaseAST(){
        id = std::string(id_);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncDef;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

// function parameter
class FuncParamAST : public BaseAST {
public:
    std::string id;
    FuncParamAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncParam;
        set_ast_type(tmpType);
    }
    explicit FuncParamAST(const char * id_):BaseAST(){
        id = std::string(id_);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncParam;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class ExpListAST : public BaseAST {
public:
    ExpListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExpList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class BlockAST : public BaseAST {
public:
    BlockAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TBlock;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class BlockItemListAST : public BaseAST{
public:
    BlockItemListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TBlockItemList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class BlockItemAST : public BaseAST{
public:
    BlockItemAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TBlockItem;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class StmtAST : public BaseAST{
public:
    StmtAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TStmt;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class IfBlockAST : public BaseAST{
public:
    IfBlockAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TIfBlock;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class WhileBlockAST : public BaseAST{
public:
    WhileBlockAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TWhileBlock;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class ExpAST : public BaseAST {
public:
    std::string id;
    int midvalue;
    ExpAST():BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        midvalue = 0;
    }
    explicit ExpAST(const char* id_):BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        id = std::string(id_);
        midvalue = 0;
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        return midvalue;
    }
};

class LOrExp : public BaseAST {
public:
    LOrExp():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TLOrExp;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class CondAST : public BaseAST {
public:
    CondAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TCond;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class LValAST : public BaseAST {
public:
    std::string id;
    LValAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TLVal;
        set_ast_type(tmpType);
    }
    explicit LValAST(const char* id_):BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TLVal;
        set_ast_type(tmpType);
        id = std::string(id_);
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        static int a = 0;
        return a;
    }
};

class NumberAST : public BaseAST{
public:
    int value;
    explicit NumberAST(int value_):BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TNumber;
        set_ast_type(tmpType);
        value = value_;
    }
    NumberAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TNumber;
        set_ast_type(tmpType);
        value = 0;
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        return value;
    }
};

class OpAST : public BaseAST{
public:
    int op;
    explicit OpAST(int op_):BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TOp;
        set_ast_type(tmpType);
        op = op_;
    }
    OpAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TOp;
        set_ast_type(tmpType);
        op = -1;
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        return op;
    }
};

// ASTPtr ASTRoot;
extern ASTPtr ASTRoot;
void attachNode(ASTPtr father, ASTPtr child);
#endif //GENY_AST_H
