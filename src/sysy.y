%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include "ast.h"

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
  BaseAST *ast;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST ASSIGN
%token <str_val> IDENT
%token <str_val> REL_OP
%token <str_val> EQ_OP
%token <str_val> LAND_OP
%token <str_val> LOR_OP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast> FuncDef FuncType Block BlockItems BlockItem Stmt 
%type <ast> Number PrimaryExp UnaryExp Exp AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <ast> BType
%type <ast> Decl ConstDecl ConstDefs ConstDef ConstInitVal LVal ConstExp

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

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
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
    auto ast = new FuncType();
    ast->func_type = "int";
    $$ = ast;
  }
  ;

Block
  : '{' BlockItems '}' {
    auto ast = new Block();
    ast->block_items = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItems
  : BlockItem ';' {
    auto ast = new BlockItems();
    ast->block_item = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | BlockItem ';' BlockItems {
    auto ast = new BlockItems();
    ast->block_item = unique_ptr<BaseAST>($1);
    ast->block_items = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

BlockItem
  : Decl {
    auto ast = new BlockItem;
    ast->decl_or_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | Stmt {
    auto ast = new BlockItem;
    ast->decl_or_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

Stmt
  : RETURN Exp {
    auto ast = new Stmt();
    ast->expr = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    auto ast = new Number();
    ast->num = $1;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExp();
    ast->unary_op = "";
    ast->unary_or_p_expr = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | '+' UnaryExp {
    auto ast = new UnaryExp();
    ast->unary_op = "+";
    ast->unary_or_p_expr = unique_ptr<BaseAST>($2);
    $$ = ast;
  } | '-' UnaryExp {
    auto ast = new UnaryExp();
    ast->unary_op = "-";
    ast->unary_or_p_expr = unique_ptr<BaseAST>($2);
    $$ = ast;
  } | '!' UnaryExp {
    auto ast = new UnaryExp();
    ast->unary_op = "!";
    ast->unary_or_p_expr = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExp();
    ast->expr_or_num_or_lval = unique_ptr<BaseAST>($2);
    $$ = ast;
  } | Number {
    auto ast = new PrimaryExp();
    ast->expr_or_num_or_lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | LVal {
    auto ast = new PrimaryExp();
    ast->expr_or_num_or_lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new Exp();
    ast->lor_expr = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExp();
    ast->mul_op = "";
    ast->unary_expr = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | MulExp '*' UnaryExp {
    auto ast = new MulExp();
    ast->mul_op = "*";
    ast->mul_expr = unique_ptr<BaseAST>($1);
    ast->unary_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  } | MulExp '/' UnaryExp {
    auto ast = new MulExp();
    ast->mul_op = "/";
    ast->mul_expr = unique_ptr<BaseAST>($1);
    ast->unary_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  } | MulExp '%' UnaryExp{
    auto ast = new MulExp();
    ast->mul_op = "%";
    ast->mul_expr = unique_ptr<BaseAST>($1);
    ast->unary_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExp();
    ast->add_op = "";
    ast->mul_expr = unique_ptr<BaseAST>($1);
  } | AddExp '+' MulExp {
    auto ast = new AddExp();
    ast->add_op = "+";
    ast->add_expr = unique_ptr<BaseAST>($1);
    ast->mul_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  } | AddExp '-' MulExp {
    auto ast = new AddExp();
    ast->add_op = "-";
    ast->add_expr = unique_ptr<BaseAST>($1);
    ast->mul_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp{
    auto ast = new RelExp();
    ast->rel_op = "";
    ast->add_expr = unique_ptr<BaseAST>($1);
  } | RelExp REL_OP AddExp {
    auto ast = new RelExp();
    ast->rel_op = *unique_ptr<string>($2);
    ast->rel_expr = unique_ptr<BaseAST>($1);
    ast->add_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp{
    auto ast = new EqExp();
    ast->eq_op = "";
    ast->rel_expr = unique_ptr<BaseAST>($1);
  } | EqExp EQ_OP RelExp {
    auto ast = new EqExp();
    ast->eq_op = *unique_ptr<string>($2);
    ast->eq_expr = unique_ptr<BaseAST>($1);
    ast->rel_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp{
    auto ast = new LAndExp();
    ast->land_op = "";
    ast->eq_expr = unique_ptr<BaseAST>($1);
  } | LAndExp LAND_OP EqExp {
    auto ast = new LAndExp();
    ast->land_op = *unique_ptr<string>($2);
    ast->land_expr = unique_ptr<BaseAST>($1);
    ast->eq_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp{
    auto ast = new LOrExp();
    ast->lor_op = "";
    ast->land_expr = unique_ptr<BaseAST>($1);
  } | LOrExp LOR_OP LAndExp {
    auto ast = new LOrExp();
    ast->lor_op = *unique_ptr<string>($2);
    ast->lor_expr = unique_ptr<BaseAST>($1);
    ast->land_expr = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LVal();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExp();
    ast->expr = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitVal();
    ast->const_expr = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDef
  : IDENT ASSIGN ConstInitVal {
    auto ast = new ConstDef();
    ast->ident = *unique_ptr<string>($1);
    ast->const_initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstDefs
  : ConstDef {
    auto ast = new ConstDefs();
    ast->const_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | ConstDef ',' ConstDefs {
    auto ast = new ConstDefs();
    ast->const_def = unique_ptr<BaseAST>($1);
    ast->const_defs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

BType
  : INT {
    auto ast = new BType();
    ast->type = "int";
    $$ = ast;
  }
  ;


ConstDecl
  : CONST BType ConstDefs {
    auto ast = new ConstDecl();
    ast->btype = unique_ptr<BaseAST>($2);
    ast->const_defs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

Decl
  : ConstDecl {
    auto ast = new Decl();
    ast->const_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  extern int yylineno;    // defined and maintained in lex
  extern char *yytext;    // defined and maintained in lex
  int len=strlen(yytext);
  int i;
  char buf[512]={0};
  for (i=0;i<len;++i)
  {
      sprintf(buf,"%s %d ",buf,yytext[i]);
  }
  fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}
