#ifndef GENY_SRC_FRONTEND_SYMBOL
#define GENY_SRC_FRONTEND_SYMBOL
#include <bits/stdc++.h>

enum symbol : int32_t {
    BREAK=300,
    CONTINUE,
    ELSE,
    INT,
    IF,
    MAIN,
    RETURN,
    WHILE,
    VOID,
    CONST,
    INT_CONSTANT,
    IDENTIFIER,
    OP_AND,
    OP_OR,
    OP_EQ,
    OP_NE,
    OP_LE,
    OP_GE,
};
#endif