#include "9cc.h"

//エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr,"%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//次のトークンが期待している記号の時には、トークンを１つ読み進めて
//真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token -> kind != TK_RESERVED || 
      strlen(op) != token->len  ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

//次のトークンが変数なら１つ読み進めて
//元のトークンを返す
//それ以外ならNULL
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *cur = token;
  token = token->next;
  return cur;  
}

//次のトークンが期待している記号の時には、トークンを１つ読み進める。
//それ以外の場合にはエラーを報告する。
void expect(char *op){
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

//次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
//それ以外の場合にはエラーを報告する。
int expect_number(){
  if(token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

// 変数として有効な文字か
int is_alnum(char c) {
  return  ('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z') ||
          ('0' <= c && c <= '9') ||
          (c == '_');
}

// 識別子の文字数を調べる
int count_ident_len(char *p) {
  int len = 0;
  while (is_alnum(*p)) {
    len++;
    p++;
  }
  return len;
}

// ローカル変数の数を返す
int count_lvar() {
  int count = 0;
  for (LVar *var = locals; var; var = var->next) {
    count++;
  }

  return count;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

Node *primary(){
  // 次のトークンが"("なら、"(" expr ")"のはず
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

Token *tok = consume_ident();
if (tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    node->offset = lvar->offset;
  } else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;

    if (locals) {
      lvar->offset = locals->offset + 8;
    } else {
      lvar->offset = 0;
    }

    node->offset = lvar->offset;
    locals = lvar;
  }
  return node;
}

  //そうでないなら数値のはず
  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}


Node *mul() {
  Node *node = unary();

  for(;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LES, node, add());
    else if (consume("<="))
      node = new_node(ND_LEE, node, add());
    else if (consume(">"))
      node = new_node(ND_LES, add(), node); 
    else if (consume(">="))
      node = new_node(ND_LEE, add(), node);
    else
      return node;
  }
}

Node *equality(){
  Node *node = relational();

  for(;;) {
    if (consume("=="))
      node = new_node(ND_EQU, node, relational());
    else if(consume("!="))
      node = new_node(ND_NOT, node, relational());
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}


Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur-> next = tok;
  return tok;
}

Token *new_token_ident(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok-> kind = kind;
  tok->str = str;
  tok->len = count_ident_len(str);
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    //空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if ( (memcmp(p, "==", 2) == 0) || (memcmp(p, "!=", 2) == 0)
        || (memcmp(p, "<=", 2) == 0) || (memcmp(p, ">=", 2) == 0)) {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
    }

    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (is_alnum(*p)) {
      cur = new_token_ident(TK_IDENT, cur, p);
      p += cur->len;
      continue;
    }

    error_at(p, "トークナイズできません");
  }
  
  new_token(TK_EOF, cur, p, 1);
  return head.next;
}