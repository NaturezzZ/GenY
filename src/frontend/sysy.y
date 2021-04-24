%{
#include <stdio.h>
#include <string.h>
#include <bits/stdc++.h>
#include "define/ast.h"
#include "define/type.h"
#include "define/util.h"

extern int lineno;
extern FILE*yyout;
void yyerror(const char*);
int yylex(void);
void gen_eeyore();
%}
%union{
	int		int_value;
	char *	string_value;
	TreeNode * node_value;
}

 // ************* Tokens ************** //
%token <int_value> INT_CONSTANT
%token <string_value> IDENT
%token INT
%token MAIN
%token BREAK CONTINUE ELSE IF WHILE VOID CONST
%token RETURN
%token OP_AND OP_OR OP_EQ OP_NE

 // ************* Left ************** //
%left OP_AND OP_OR
%left OP_EQ OP_NE
%left OP_LE OP_GE
%left '<' '>'
%left '+' '-'
%left '*' '/' '%'
%nonassoc	LOWER_THAN_ELSE
%nonassoc	ELSE

 // ************* Start ************** //
%start Program

%%
Program :   CompUnit { }
		;

CompUnit:   Decl { }
		|   FuncDef { }
		|   CompUnit Decl { }
		|   CompUnit FuncDef { }
		;

Decl    :   ConstDecl { }
		|   VarDecl { }
		;

ConstDecl   :   CONST INT ConstDef_list ';' { }
			;

// const definitions, mutiple is allowed
// ConstDef { ',' ConstDef }
ConstDef_list   :   ConstDef { }
				|   ConstDef_list ',' ConstDef { }
				;

ConstDef    :   IDENT Dimensions_list '=' ConstInitVal { }
			;

// list of dimensions, [3][2], [1]
// { '[' ConstExp ']' }
Dimensions_list :   '[' ConstExp ']' { }
				|   Dimensions_list '[' ConstExp ']' { }
				;

ConstInitVal    :   ConstExp {}
				|   '{' '}' {}
				|   '{' ConstInitVal_list '}' {}
				;

ConstInitVal_list   :   ConstInitVal {}
					|   ConstInitVal_list ',' ConstInitVal {}
					;

VarDecl :   INT VarDef_list ';' {}
		;

// VarDef { ',' VarDef }
// eg. 1 1,2 1,2,3
VarDef_list :   VarDef {}
			|   VarDef_list ',' VarDef {}
			;

VarDef  :   IDENT Dimensions_list {}
		|   IDENT Dimensions_list '=' InitVal {}
		;

InitVal :   Exp {}
		|   '{' '}' {}
		|   '{' InitVal_list '}' {}
		;

// InitVal { ',' InitVal }
// 1,2,3,4 1
InitVal_list    :   InitVal {}
				|   InitVal_list ',' InitVal {}
				;

FuncDef :   VOID IDENT '(' ')' Block {}
		|   VOID IDENT '(' FuncFParams ')' Block {}
		|   INT IDENT '(' ')' Block {}
        |   INT IDENT '(' FuncFParams ')' Block {}
		;

// FuncType    :   VOID {}
// 			|   INT {}
// 			;

FuncFParams :   FuncFParam {}
			|   FuncFParams ',' FuncFParam {}
			;

FuncFParam  :   INT IDENT {}
			|   INT IDENT '[' ']' Exp_list {}
			;

// { '[' Exp ']' }
Exp_list    :   '[' Exp ']' Exp_list  {}
			|   /*empty*/ {}
			;

Block   :   '{' BlockItem_list '}' {}
		;

BlockItem_list  :   BlockItem BlockItem_list {}
				|   /*empty*/ {}
				;

BlockItem   :   Decl {}
			|   Stmt {}
			;

// Stmt
// 	: Matched_Stmt {}
// 	| Open_Stmt {}
// 	;
//
// Matched_Stmt
// 	: IF '(' Cond ')' Matched_Stmt ELSE Matched_Stmt {}
// 	| NIf_Stmt {}
// 	;
//
// Open_Stmt
// 	: IF '(' Cond ')' Stmt
// 	| IF '(' Cond ')' Matched_Stmt ELSE Open_Stmt {}
//
// NIf_Stmt
// 	: LVal '=' Exp ';' {}
//     | ';' {}
//     | Exp ';' {}
//     | Block {}
//     | WHILE '(' Cond ')' Stmt {}
//     | BREAK ';' {}
//     | CONTINUE ';' {}
//     | RETURN ';' {}
//     | RETURN Exp ';' {}
//     ;

Stmt
 	: LVal '=' Exp ';' {}
 	| IF '(' Cond ')' Stmt ELSE Stmt {}
 	| IF '(' Cond ')' Stmt {} %prec LOWER_THAN_ELSE
    | ';' {}
    | Exp ';' {}
    | Block {}
    | WHILE '(' Cond ')' Stmt {}
    | BREAK ';' {}
    | CONTINUE ';' {}
    | RETURN ';' {}
    | RETURN Exp ';' {}
    ;

Exp :   AddExp {}
	;

Cond    :   LOrExp {}
		;

LVal    :   IDENT Exp_list {}
		;

PrimaryExp  :   '(' Exp ')' {}
			|   LVal {}
			|   Number {}
			;

Number  :   INT_CONSTANT {}
		;

UnaryExp    :   PrimaryExp {}
			|   IDENT '(' ')' {}
			|   IDENT '(' FuncRParams ')' {}
			|   UnaryOp UnaryExp {}
			;

UnaryOp :   '+' {}
		|   '-' {}
		|   '!' {}
		;

FuncRParams :   Exp {}
			|   FuncRParams ',' Exp {}
			;

MulExp  :   UnaryExp {}
		|   MulExp Mul_Div_Mod UnaryExp {}
		;

Mul_Div_Mod :   '*' {}
			|   '/' {}
			|   '%' {}
			;

AddExp  :   MulExp {}
		|   AddExp Plus_Minus MulExp {}
		;

Plus_Minus  :   '+' {}
			|   '-' {}
			;

RelExp  :   AddExp {}
		|   RelExp Rel AddExp {}
		;

Rel :   '<' {}
	|   '>' {}
	|   OP_LE {}
	|   OP_GE {}
	;

EqExp   :   RelExp {}
		|   EqExp EqN RelExp {}
		;

EqN  :   OP_EQ {}
	|   OP_NE {}
	;

LAndExp :   EqExp {}
		|   LAndExp OP_AND EqExp {}
		;

LOrExp  :   LAndExp {}
		|   LOrExp OP_OR LAndExp {}
		;

ConstExp    :   AddExp {}
			;

%%
/*  Cpp Code Starts */

/*  Cpp Code Ends   */



/****************************************/
/************* Reference ****************/
/****************************************/