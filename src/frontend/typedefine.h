//
// Created by Naiqian on 2021/4/18.
//

#ifndef GENY_TYPEDEFINE_H
#define GENY_TYPEDEFINE_H

//#define __DEBUG__
#ifdef __DEBUG__
#define dprintf(format,...)  fprintf(stderr,format,##__VA_ARGS__)
#else
#define dprintf(format,...)
#endif

typedef enum NodeKind {
    IDEN, 	//identifier
    NUM, 	//integer
    EXP,	//expression
    CALLP,	//callparam
    RETURN_,	//return
    FUNCCALL, 	//function call
    GOAL,	//goal of program
    DEFN,
    VARDEFN,
    FUNCDEFN,
    FUNCDECL,
    ASSIGNMENT,
    MAINFUNC,
    PARAMDECL,
    VARDECL,
    FUNCCONTEXT,
    STATEMENTS,
    STMT,
} NodeKind;
typedef enum StmtKind {
    kong,
    statements_,
    if_,
    if_else,
    while_,
    assign1,
    assign2,
    vardefn_,
    return_,
    funccall_, //
} StmtKind;
typedef enum ExpKind {
    add,
    minus,
    multiply,
    divide,
    mod,
    op_and,
    op_or,
    op_less,
    op_greater,
    op_eq,
    op_ne,
    op_index,
    integer,
    identifier,
    op_judge,  //
    op_neg,    //
    bracket,   //
    funccall,
} ExpKind;
typedef struct TokenType {
    int c;
} TokenType;
typedef struct ExpType {
    int d;
} ExpType;

#define MAXCHILDREN 100
#define MAXTREENODE 2000
typedef struct TreeNode {
    struct TreeNode* child[MAXCHILDREN];
    int childcnt;
    struct TreeNode* sibling;
    int lineno;
    NodeKind nodekind;
    union{ StmtKind stmt; ExpKind exp;} kind;
    union{ TokenType op;
        int val;
        char* name;} attr;
    ExpType type; //for type checking of exps
} TreeNode;

#endif //GENY_TYPEDEFINE_H
