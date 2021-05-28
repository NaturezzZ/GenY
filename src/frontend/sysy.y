
%code requires
{
    #include <stdio.h>
    #include <string.h>

    #include "define/ast.h"
    #include "define/type.h"
    #include "define/util.h"
    #include "define/symtab.h"

    extern int lineno;
    extern FILE*yyout;
    void yyerror(const char*);
    //int yylex(void);
    extern "C"
    {
        //void yyerror(const char *s);
        extern int yylex(void);
    }
    extern BaseAST* ASTRoot;
}

%{

%}

%union {
    int		int_value;
    char *	string_value;
    ASTPtr	node;
};

 // ************* Tokens ************** //
%token <int_value> INT_CONSTANT
%token <string_value> IDENT
%token INT
%token MAIN
%token BREAK CONTINUE ELSE IF WHILE VOID CONST
%token RETURN
%token OP_AND OP_OR OP_EQ OP_NE
%type <node> Program CompUnit Decl ConstDecl ConstDef_list VarDef_list ConstDef Dimensions_list
%type <node> ConstInitVal ConstInitVal_list VarDecl InitVal_list FuncDef FuncFParams
%type <node> FuncFParam Exp_list Block BlockItem_list BlockItem Exp Cond LVal PrimaryExp
%type <node> Number UnaryExp FuncRParams MulExp Mul_Div_Mod AddExp Plus_Minus 
%type <node> IfBlock WhileBlock VarDef InitVal
%type <node> RelExp Rel EqExp EqN LAndExp LOrExp ConstExp 
%type <node> UnaryOp Stmt

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
Program:   
    CompUnit {
        std::cout << "present" << std::endl;
        ASTRoot = new RootAST();
        $$ = ASTRoot;
        ASTRoot->father = ASTRoot;
        attachNode(ASTRoot, $1);
    }
    ;

CompUnit:   
    Decl {
        $$ = new CompUnitAST();
        attachNode($$, $1);
        $$->ast_type()->withDecl = true;
    }
    | FuncDef { 
        $$ = new CompUnitAST();
        attachNode($$, $1);
        $$->ast_type()->withFuncDef = true;
    }
    | CompUnit Decl {
        $$ = new CompUnitAST();
        attachNode($$, $1);
        attachNode($$, $2);
        $$->ast_type()->withCompUnit = true;
        $$->ast_type()->withDecl = true;
    }
    | CompUnit FuncDef { 
        $$ = new CompUnitAST();
        attachNode($$, $1);
        attachNode($$, $2);
        $$->ast_type()->withCompUnit = true;
        $$->ast_type()->withFuncDef = true;
    }
    ;

Decl:
    ConstDecl { 
        $$ = new DeclAST();
        $$->ast_type()->isConstDecl = true;
        attachNode($$, $1);
    }
    | VarDecl { 
        $$ = new DeclAST();
        $$->ast_type()->isConstDecl = false;
        attachNode($$, $1);
    }
    ;

ConstDecl:
    CONST INT ConstDef_list ';' { 
        $$ = $3;
    }
    ;

// const definitions, mutiple is allowed
// ConstDef { ',' ConstDef }
ConstDef_list:
    ConstDef { 
        $$ = new DefListAST();
        $$->ast_type()->isConstDefList = true;
        $$->ast_type()->isDest = true;
        attachNode($$, $1);
    }
    | ConstDef_list ',' ConstDef { 
        $$ = new DefListAST((DefListAST*)$1);
        ((DefListAST*)$$)->addMember((DefAST *)($3));
        $$->ast_type()->isConstDefList = true;
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

ConstDef:
    IDENT '=' ConstInitVal { 
        $$ = new DefAST($1);
        $$->isArray = false;
        $$->withInitVal = true;
        $$->isConst = true;
        attachNode($$, $3);
    }
    | IDENT Dimensions_list '=' ConstInitVal { 
        $$ = new DefAST($1);
        $$->isArray = true;
        $$->withInitVal = true;
        $$->isConst = true;
        attachNode($$, $2);
        attachNode($$, $4);
    }
    ;

// list of dimensions, [3][2], [1]
// { '[' ConstExp ']' }
Dimensions_list:
    '[' ConstExp ']' { 
        $$ = new DimensionsListAST();
        // ((DimensionsListAST*)$$)->addMember(($2)->getValueInt());
        attachNode($$, $2);
    }
    | Dimensions_list '[' ConstExp ']' { 
        $$ = new DimensionsListAST();
        // ((DimensionsListAST*)$$)->addMember(($1)->getValueVectorInt());
        // ((DimensionsListAST*)$$)->addMember(($3)->getValueInt());
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

ConstInitVal:
    ConstExp {
        $$ = new NestListAST();
        // int tmp = ($1)->getValueInt();
        // $$->addMember(tmp);
        attachNode($$, $1);
    }
    | '{' '}' {
        $$ = new NestListAST();
        // attachNode($$, $1);
    }
    |'{' ConstInitVal_list '}' {
        $$ = new NestListAST();
        // $$->addMember(($2)->getValueVectorInt());
        attachNode($$, $2);
    }
    ;

ConstInitVal_list:
    ConstInitVal {
        $$ = new NestListAST();
        attachNode($$, $1);
    }
    | ConstInitVal_list ',' ConstInitVal {
        $$ = new NestListAST();
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

VarDecl:
    INT VarDef_list ';' {
        $$ = $2;
    }
    ;

// VarDef { ',' VarDef }
// eg. 1 1,2 1,2,3
VarDef_list:
    VarDef {
        $$ = new DefListAST();
        $$->ast_type()->isConstDefList = false;
        $$->ast_type()->isDest = true;
        attachNode($$, $1);
    }
    | VarDef_list ',' VarDef {
        $$ = new DefListAST((DefListAST*)$1);
        ((DefListAST*)$$)->addMember((DefAST *)$3);
        $$->ast_type()->isConstDefList = false;
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

VarDef:
    IDENT Dimensions_list {
        $$ = new DefAST($1);
        $$->isArray = true;
        $$->withInitVal = false;
        $$->isConst = false;
        attachNode($$, $2);
    }
    | IDENT Dimensions_list '=' InitVal {
        $$ = new DefAST($1);
        $$->isArray = true;
        $$->withInitVal = true;
        $$->isConst = false;
        attachNode($$, $2);
        attachNode($$, $4);
    }
    | IDENT {
        $$ = new DefAST($1);
        $$->isArray = false;
        $$->withInitVal = false;
        $$->isConst = false;
    }
    | IDENT '=' InitVal {
        $$ = new DefAST($1);
        $$->isArray = false;
        $$->withInitVal = true;
        $$->isConst = false;
        attachNode($$, $3);
    }
    ;

InitVal:
    Exp {
        $$ = new NestListAST();
        attachNode($$, $1);
    }
    | '{' '}' {
        $$ = new NestListAST();
    }
    | '{' InitVal_list '}' {
        $$ = new NestListAST();
        attachNode($$, $2);
    }
    ;

// InitVal { ',' InitVal }
// 1,2,3,4 1
InitVal_list:
    InitVal {
        $$ = new NestListAST();
        $$->ast_type()->isConstInitList = false;
        attachNode($$, $1);
    }
    | InitVal_list ',' InitVal {
        $$ = new NestListAST();
        $$->ast_type()->isConstInitList = false;
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

FuncDef:
    VOID IDENT '(' ')' Block {
        $$ = new FuncDefAST($2);
        $$->ast_type()->isReturnInt = false;
        $$->ast_type()->withFuncFParam = false;
        attachNode($$, $5);
    }
    | VOID IDENT '(' FuncFParams ')' Block {
        $$ = new FuncDefAST($2);
        $$->ast_type()->isReturnInt = false;
        $$->ast_type()->withFuncFParam = true;
        attachNode($$, $4);
        attachNode($$, $6);
    }
    | INT IDENT '(' ')' Block {
        $$ = new FuncDefAST($2);
        $$->ast_type()->isReturnInt = true;
        $$->ast_type()->withFuncFParam = false;
        attachNode($$, $5);
    }
    | INT IDENT '(' FuncFParams ')' Block {
        $$ = new FuncDefAST($2);
        $$->ast_type()->isReturnInt = true;
        $$->ast_type()->withFuncFParam = true;
        attachNode($$, $4);
        attachNode($$, $6);
    }
    ;

// FuncType    :   VOID {}
// 			|   INT {}
// 			;

FuncFParams:
    FuncFParam {
        $$ = new FuncParamAST();
        attachNode($$, $1);
    }
    | FuncFParams ',' FuncFParam {
        $$ = new FuncParamAST();
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

FuncFParam: 
    INT IDENT {
        $$ = new FuncParamAST($2);
    }
    | INT IDENT '[' ']' Exp_list {
        $$ = new FuncParamAST($2);
        attachNode($$, $5);
    }
    ;

// { '[' Exp ']' }
Exp_list:
    '[' Exp ']' Exp_list  {
        $$ = new ExpListAST();
        attachNode($$, $2);
        attachNode($$, $4);
    }
    | /*empty*/ {
        $$ = new ExpListAST();
        $$->ast_type()->isEmpty = true;
    }
    ;

Block:
    '{' BlockItem_list '}' {
        $$ = new BlockAST();
        attachNode($$, $2);
    }
    ;

BlockItem_list:
    BlockItem BlockItem_list {
        $$ = new BlockItemListAST();
        attachNode($$, $1);
        attachNode($$, $2);
    }
    | 
    {
        $$ = new BlockItemListAST();
        $$->ast_type()->isEmpty = true;
    }
    ;

BlockItem:
    Decl {
        $$ = new BlockItemAST();
        attachNode($$, $1);
    }
    | Stmt {
        $$ = new BlockItemAST();
        attachNode($$, $1);
    }
    ;

Stmt: 
    IfBlock {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STIF;
        attachNode($$, $1);
    }
    | Block {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STBLOCK;
        attachNode($$, $1);
    }
    | WhileBlock {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STWHILE;
        attachNode($$, $1);
    }
    | LVal '=' Exp ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STASSIGN;
        attachNode($$, $1);
        attachNode($$, $3);
    }
    | ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STSEMI;
    }
    | Exp ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STEXP;
        attachNode($$, $1);
    }
    | BREAK ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STBREAK;
        $$->ast_type()->isBreak = true;
    }
    | CONTINUE ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STCONTINUE;
        $$->ast_type()->isContinue = true;
    }
    | RETURN ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STRETURN;
        $$->ast_type()->isReturn = true;
    }
    | RETURN Exp ';' {
        $$ = new StmtAST();
        ((StmtAST*)$$)->type = STRETURNEXP;
        $$->ast_type()->isReturn = true;
        $$->ast_type()->withReturnValue = true;
        attachNode($$, $2);
    }
    ;

IfBlock:
    IF '(' Cond ')' Stmt ELSE Stmt {
        $$ = new IfBlockAST();
        attachNode($$, $3);
        attachNode($$, $5);
        attachNode($$, $7);
    }
    | IF '(' Cond ')' Stmt %prec LOWER_THAN_ELSE {
        $$ = new IfBlockAST();
        attachNode($$, $3);
        attachNode($$, $5);
    }
    ;

WhileBlock:
    WHILE '(' Cond ')' Stmt {
        $$ = new WhileBlockAST();
        attachNode($$, $3);
        attachNode($$, $5);
    }
    ;

Exp:
    AddExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    ;

Cond:
    LOrExp {
        $$ = new CondAST();
        attachNode($$, $1);
    }
    ;

LVal:
    IDENT Exp_list {
        $$ = new LValAST($1);
        attachNode($$, $2);
    }
    ;

PrimaryExp:
    '(' Exp ')' {
        $$ = new PrimaryExpAST();
        $$->type = 0;
        attachNode($$, $2);
    }
    | LVal {
        $$ = new PrimaryExpAST();
        $$->type = 1;
        attachNode($$, $1);
    }
    | Number {
        $$ = new PrimaryExpAST();
        $$->type = 2;
        attachNode($$, $1);
    }
    ;

Number:
    INT_CONSTANT {
        $$ = new NumberAST($1);
    }
    ;

UnaryExp:
    PrimaryExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | IDENT '(' ')' {
        $$ = new ExpAST($1);
    }
    | IDENT '(' FuncRParams ')' {
        $$ = new ExpAST($1);
        attachNode($$, $3);
    }
    | UnaryOp UnaryExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        attachNode($$, $2);
    }
    ;

UnaryOp:
    '+' {
        $$ = new OpAST('+');
    }
    | '-' {
        $$ = new OpAST('-');
    }
    | '!' {
        $$ = new OpAST('!');
    }
    ;

FuncRParams:
    Exp {
        $$ = new FuncParamAST();
        attachNode($$, $1);
    }
    | FuncRParams ',' Exp {
        $$ = new FuncParamAST();
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

MulExp:
    UnaryExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | MulExp Mul_Div_Mod UnaryExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        attachNode($$, $2);
        attachNode($$, $3);
    }
    ;

Mul_Div_Mod:
    '*' {
        $$ = new OpAST('*');
    }
    | '/' {
        $$ = new OpAST('/');
    }
    | '%' {
        $$ = new OpAST('%');
    }
    ;

AddExp:
    MulExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | AddExp Plus_Minus MulExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        attachNode($$, $2);
        attachNode($$, $3);
    }
    ;

Plus_Minus:   
    '+' {
        $$ = new OpAST('+');
    }
    | '-' {
        $$ = new OpAST('-');
    }
    ;

RelExp:
    AddExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | RelExp Rel AddExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        attachNode($$, $2);
        attachNode($$, $3);
    }
    ;

Rel:
    '<' {
        $$ = new OpAST('<');
    }
    | '>' {
        $$ = new OpAST('>');
    }
    | OP_LE {
        $$ = new OpAST(BOP_LE);
    }
    | OP_GE {
        $$ = new OpAST(BOP_GE);
    }
    ;

EqExp:   
    RelExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | EqExp EqN RelExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        attachNode($$, $2);
        attachNode($$, $3);
    }
    ;

EqN:
    OP_EQ {
        $$ = new OpAST(BOP_EQ);
    }
    | OP_NE {
        $$ = new OpAST(BOP_NE);
    }
    ;

LAndExp:
    EqExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | LAndExp OP_AND EqExp {
        $$ = new ExpAST();
        // $$->getValueInt() = BOP_AND;
        attachNode($$, $1);
        attachNode($$, $3);
    }
    ;

LOrExp:
    LAndExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
    | LOrExp OP_OR LAndExp {
        $$ = new ExpAST();
        attachNode($$, $1);
        // attachNode($$, $2);
        attachNode($$, $3);
    }
    ;

ConstExp:
    AddExp {
        $$ = new ExpAST();
        attachNode($$, $1);
    }
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
    //yyparse();
}
 //int main(){
//     yyparse();
//     return 0;
// }
/*  Cpp Code Ends   */



/****************************************/
/************* Reference ****************/
/****************************************/