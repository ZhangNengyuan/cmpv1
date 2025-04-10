%code requires {
  #include "ast.hpp"
  #include <memory>
  #include <string>
}

%{
#include "ast.hpp"
#include <iostream>
#include <memory>
#include <string>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST
%token <str_val> IDENT EQOP RELOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt UnaryExp Exp PrimaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl BType ConstDef ConstInitVal BlockItem LVal ConstExp
%type <int_val> Number

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Decl
  : ConstDecl{
    $$ = $1;
  }
  ;

ConstDecl
  : CONST INT ConstDef 

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')'{
    auto ast = new PrimaryExpAST();
    ast -> isnumber = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  |Number{
    auto ast = new PrimaryExpAST();
    ast -> isnumber = 1;
    ast->number = std::make_unique<NumberAST>($1);
    $$ = ast;
  }
  ;


UnaryExp
  : PrimaryExp{
    auto ast = new UnaryExpAST();
    ast -> isprimary = 1;
    ast->primaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |'+' UnaryExp{
    auto ast = new UnaryExpAST();
    ast -> isprimary = 0;
    ast -> op = '+';
    ast -> unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|'-' UnaryExp{
    auto ast = new UnaryExpAST();
    ast -> isprimary = 0;
    ast -> op = '-';
    ast -> unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|'!' UnaryExp{
    auto ast = new UnaryExpAST();
    ast -> isprimary = 0;
    ast -> op = '!';
    ast -> unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp{
    auto ast = new MulExpAST();
    ast -> only = 1;
    ast -> unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;

  }
  |MulExp '%' UnaryExp{
    auto ast = new MulExpAST();
    ast -> only = 0;
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> op = '%';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|MulExp '*' UnaryExp{
    auto ast = new MulExpAST();
    ast -> only = 0;
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> op = '*';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|MulExp '/'UnaryExp{
    auto ast = new MulExpAST();
    ast -> only = 0;
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> op = '/';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp{
    auto ast = new AddExpAST();
    ast -> only = 1;
    ast -> mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |AddExp '-' MulExp{
    auto ast = new AddExpAST();
    ast -> only = 0;
    ast -> addexp = unique_ptr<BaseAST>($1);
    ast -> op = '-';
    ast -> mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|AddExp '+' MulExp{
    auto ast = new AddExpAST();
    ast -> only = 0;
    ast -> addexp = unique_ptr<BaseAST>($1);
    ast -> op = '+';
    ast -> mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp{
    auto ast = new RelExpAST();
    ast -> only =1;
    ast -> addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |RelExp RELOP AddExp{
    auto ast = new RelExpAST();
    ast -> only =0;
    ast -> relexp = unique_ptr<BaseAST>($1);
    ast->op = *($2);
    ast -> addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
    
  }
  ;

EqExp
  : RelExp{
    auto ast = new EqExpAST();
    ast -> only =1;
    ast -> relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |EqExp EQOP RelExp{
    auto ast = new EqExpAST();
    ast -> only =0;
    ast -> eqexp = unique_ptr<BaseAST>($1);
    ast->op = *($2);
    ast -> relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
    
  }
  ;

LAndExp
  : EqExp{
    auto ast = new LAndExpAST();
    ast -> only =1;
    ast -> eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |LAndExp '&''&' EqExp{
    auto ast = new LAndExpAST();
    ast -> only =0;
    ast -> landexp = unique_ptr<BaseAST>($1);
    ast -> eqexp = unique_ptr<BaseAST>($4);
    $$ = ast;
    
  }
  ;

LOrExp
  : LAndExp{
    auto ast = new LOrExpAST();
    ast -> only =1;
    ast -> landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |LOrExp '|''|' LAndExp{
    auto ast = new LOrExpAST();
    ast -> only =0;
    ast -> lorexp = unique_ptr<BaseAST>($1);
    ast -> landexp = unique_ptr<BaseAST>($4);
    $$ = ast;
    
  }
  ;



%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
