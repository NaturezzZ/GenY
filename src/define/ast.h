//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_AST_H
#define GENY_AST_H
#include "define/type.h"
#include <bits/stdc++.h>
#include "define/symtab.h"
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
        children = std::vector<ASTPtr>(obj.children);
        siblings = std::vector<ASTPtr>(obj.siblings);
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
    std::map<std::string, initValue> decls; // id, init_val
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
    std::vector<DefAST*> members;
    uint32_t addMember(DefAST * def){
        members.push_back(def);
        return static_cast<uint32_t>(members.size());
    }
    explicit DefListAST(const DefListAST* obj):BaseAST(*obj){
        members = std::vector<DefAST*>(obj->members);
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
    bool isArray;
    bool withInitVal;
    bool isConst;

    DefAST(const DefAST &obj):BaseAST(obj) {
        id = std::string(obj.id);
        dimensions = std::vector<int>(obj.dimensions);
        initval = std::vector<int>(initval);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
        isArray = obj.isArray;
        withInitVal = obj.withInitVal;
        isConst = obj.isConst;
    }

    DefAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
        isArray = false;
        withInitVal = false;
        isConst = false;
    }
    explicit DefAST(const std::string & id_):BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TDef;
        set_ast_type(tmpType);
        id = id_;
        isArray = false;
        withInitVal = false;
        isConst = false;
    }
    /*
//    DefAST(const std::string & id_, const std::vector<int> & dimensions_, const std::vector<int> & initval_):BaseAST(){
//        id = std::string(id_);
//        dimensions = std::vector<int>(dimensions_);
//        initval = std::vector<int>(initval_);
//        TypePtr tmpType = std::make_shared<BaseType>();
//        tmpType.get()->type = TDef;
//        set_ast_type(tmpType);
//    }
//
//    DefAST(const std::string & id_, const std::vector<int> & dimensions_):BaseAST(){
//        id = std::string(id_);
//        dimensions = std::vector<int>(dimensions_);
//        initval = std::vector<int>(dimensions.size());
//        TypePtr tmpType = std::make_shared<BaseType>();
//        tmpType.get()->type = TDef;
//        set_ast_type(tmpType);
//    }
     */
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
        return static_cast<unsigned int>(value.size());
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
    std::vector<int> data;
    unsigned addMember(ASTPtr p);
    NestListAST(const NestListAST & obj):BaseAST(obj){
        value = ASTPtrList(obj.value);
    }
    NestListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TNestList;
        set_ast_type(tmpType);
    }
    std::vector<int> & getValueVectorInt() override{
        return data;
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
    int pNum; // number of params
    FuncDefAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncDef;
        set_ast_type(tmpType);
        pNum = 0;
    }

    explicit FuncDefAST(const std::string & id_):BaseAST(){
        id = std::string(id_);
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TFuncDef;
        set_ast_type(tmpType);
        pNum = 0;
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
    int value;
    ExpListAST():BaseAST(){
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExpList;
        value = 0;
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
    int type;
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
    int value;
    ExpAST():BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        value = 0;
    }
    explicit ExpAST(const char* id_):BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        id = std::string(id_);
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

class PrimaryExpAST : public BaseAST {
public:
    std::string id;
    int value;
    int type;
    PrimaryExpAST():BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        value = 0;
        type = 0;
    }
    explicit PrimaryExpAST(const char* id_):BaseAST() {
        TypePtr tmpType = std::make_shared<BaseType>();
        tmpType.get()->type = TExp;
        set_ast_type(tmpType);
        id = std::string(id_);
        value = 0;
        type = 0;
    }
    std::vector<int> & getValueVectorInt() override{
        static std::vector<int> a;
        return a;
    }
    int & getValueInt() override{
        return value;
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
    int value;
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

void dispatchRoot(ASTPtr treeRoot);
void dispatchCompUnit(ASTPtr treeRoot);
void dispatchDecl(ASTPtr treeRoot);
void dispatchDefList(ASTPtr treeRoot);
void dispatchConstDefList(ASTPtr treeRoot);
void dispatchConstDef(ASTPtr treeRoot);
void dispatchDef(ASTPtr treeRoot);
std::vector<retVal_t> dispatchDimensionsList(ASTPtr treeRoot);

std::string dispatchConstInitVal(ASTPtr treeRoot, std::vector<retVal_t>& target);
std::string dispatchConstInitList(ASTPtr treeRoot, std::vector<retVal_t>& target);
std::string dispatchInitVal(ASTPtr treeRoot, std::vector<retVal_t>& target);
std::string dispatchInitList(ASTPtr treeRoot, std::vector<retVal_t>& target);

retVal_t dispatchConstExp(ASTPtr treeRoot);
retVal_t dispatchAddExp(ASTPtr treeRoot);
retVal_t dispatchMulExp(ASTPtr treeRoot);
retVal_t dispatchUnaryExp(ASTPtr treeRoot);
retVal_t dispatchPrimaryExp(ASTPtr treeRoot);
void dispatchFuncDef(ASTPtr treeRoot);
int dispatchFuncFParamList(ASTPtr treeRoot);
void dispatchFuncFParam(ASTPtr treeRoot, int paramIndex);
int getFuncParamNumber(ASTPtr treeRoot);
std::vector<retVal_t> dispatchFuncRParamList(ASTPtr treeRoot);
std::vector<retVal_t> dispatchExpList(ASTPtr treeRoot);
void dispatchBlock(ASTPtr treeRoot);
void dispatchBlockItemList(ASTPtr treeRoot);
void dispatchBlockItem(ASTPtr treeRoot);
void dispatchStmt(ASTPtr treeRoot);
void dispatchIfBlock(ASTPtr treeRoot);
void dispatchWhileBlock(ASTPtr treeRoot);
retVal_t dispatchExp(ASTPtr treeRoot);
retVal_t dispatchLOrExp(ASTPtr treeRoot);
retVal_t dispatchLAndExp(ASTPtr treeRoot);
retVal_t dispatchEqExp(ASTPtr treeRoot);
retVal_t dispatchRelExp(ASTPtr treeRoot);
retVal_t dispatchCond(ASTPtr treeRoot);
retVal_t dispatchLVal(ASTPtr treeRoot);

// ASTPtr ASTRoot;
extern ASTPtr ASTRoot;
void attachNode(ASTPtr father, ASTPtr child);
void asterr(const char* s);
void process_array(std::vector<retVal_t>& src, std::vector<retVal_t>& dst, std::string& pattern, std::vector<int>& dims);

class varInfo{
    std::string id;

};
#endif //GENY_AST_H
