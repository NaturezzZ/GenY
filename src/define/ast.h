//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_AST_H
#define GENY_AST_H
#include "frontend/sysy.tab.h"
#include "define/type.h"

class BaseAST {
public:
    virtual ~BaseAST() = default;
    // return true if current AST is a literal value
    virtual bool IsLiteral() const = 0;
    // return true if current AST is a initializer list
    virtual bool IsInitList() const = 0;

    // dump the content of AST (XML format) to output stream
    virtual void Dump(std::ostream &os) const = 0;

    const TypePtr &set_ast_type(const TypePtr &ast_type) {
        return ast_type_ = ast_type;
    }

    // getters
    const TypePtr &ast_type() const { return ast_type_; }

private:
    TypePtr ast_type_;
};

// (const) variable declaration
class VarDeclAST : public BaseAST {

};
// (const) variable definition
class VarDefAST : public BaseAST {

};
// initializer list (for array initialization)
class InitListAST : public BaseAST {

};
// function declaration
class FuncDeclAST : public BaseAST {

};
// function definition
class FuncDefAST : public BaseAST {

};
// function parameter
// NOTE: if parameter is an array, 'arr_lens_' must not be empty
//       but it's first element can be 'nullptr' (e.g. int arg[])
class FuncParamAST : public BaseAST {

};
#endif //GENY_AST_H
