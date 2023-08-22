//________________________________________
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//________________________________________
//型定義(Type definition)
//________________________________________

//トークンの種類
typedef enum{
  TK_RESERVED,  //記号
  TK_IDENT,     //識別子
  TK_NUM,       //整数トークン
  TK_EOF,       //入力の終わりを表すトークン
  TK_RETURN,    //リターントークン
  TK_IF,        //ifトークン
  TK_ELSE,      //elseトークン
  TK_WHILE,     //whileトークン
  TK_FOR,       //forトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token{
  TokenKind kind; //  トークンの型
  Token *next;    //  次の入力トークン
  int val;        //  kindがTK_NUMの場合、その数値
  char *str;      //  トークン文字列
  int len;        //  トークンの長さ
};

//抽象構文木のノードの種類
typedef enum{
  ND_ADD,   // +
  ND_SUB,   // -
  ND_MUL,   // *
  ND_DIV,   // /
  ND_NUM,   // 整数
  ND_LES,   // <
  ND_LEE,   // <=
  ND_EQU,   // ==
  ND_NOT,   // != 
  ND_ASSIGN,// =
  ND_LVAR,  // ローカル変数 
  ND_RETURN,// return
  ND_IF,    // if
  ND_WHILE, // while
  ND_FOR,   // for
  ND_ELSE,  // else
} NodeKind;


typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
  NodeKind kind;    // ノードの型
  Node *lhs;        // 左辺
  Node *rhs;        // 右辺
  int val;          // kindがND_NUMの場合のみ使う
  int offset;       // kindがND_LVARの場合のみ使う

  // "if" ( cond ) then "else" els
  // "for" ( init; cond; inc ) body
  // "while" ( cond ) body
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;
  Node *body;
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

//________________________________________
//プロトタイプ宣言(prototype definition)
//________________________________________
Token *tokenize(char *p);
Node *expr();
void gen(Node *node);
void program();
// ローカル変数の数を返す
int count_lvar();
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);




//________________________________________
//マクロ定義(Macro definition)
//________________________________________

// 入力プログラム
extern char *user_input;
//現在着目しているトークン
extern Token *token;
//コード全体
extern Node *code[100];
//ローカル変数
extern LVar *locals;

//________________________________________
