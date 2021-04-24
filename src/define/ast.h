//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_AST_H
#define GENY_AST_H
#include "frontend/sysy.tab.h"
#include "define/type.h"
#include <bits/stdc++.h>

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
    const front::Logger &logger() const { return logger_; }
    const TypePtr &ast_type() const { return ast_type_; }

private:
    front::Logger logger_;
    TypePtr ast_type_;
};
#endif //GENY_AST_H
