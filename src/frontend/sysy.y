%{
#include <stdio.h>
#include <string.h>

#include "define/ast.h"
#include "define/type.h"
#include "define/util.h"

extern int lineno;
extern FILE*yyout;
void yyerror(const char*);
//int yylex(void);
extern "C"//为了能够在C++程序里面调用C函数，必须把每一个需要使用的C函数，其声明都包括在extern "C"{}块里面，这样C++链接时才能成功链接它们。extern "C"用来在C++环境下设置C链接类型。
{   //lex.l中也有类似的这段extern "C"，可以把它们合并成一段，放到共同的头文件main.h中
    //void yyerror(const char *s);
    extern int yylex(void);//该函数是在lex.yy.c里定义的，yyparse()里要调用该函数，为了能编译和链接，必须用extern加以声明
}
%}
%union{
	int		int_value;
	char *	string_value;
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
void yyerror(const char *s)
{
	extern int yylineno;	// defined and maintained in lex
	extern char *yytext;	// defined and maintained in lex
	int len=strlen(yytext);
	int i;
	char buf[512]={0};
	for (i=0;i<len;++i)
	{
		sprintf(buf,"%s%d ",buf,yytext[i]);
	}
	fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
	yyparse();
}
/*  Cpp Code Ends   */



/****************************************/
/************* Reference ****************/
/****************************************/