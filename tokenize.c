#include "9cc.h"

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

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur-> next = tok;
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

    if (strncmp(p, "return", 6) == 0 && count_ident_len(p) == 6) {

      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (is_alnum(*p)) {
      cur = new_token(TK_IDENT, cur, p, count_ident_len(p));
      p += cur->len;
      continue;
    }

    error_at(p, "トークナイズできません");
  }
  
  new_token(TK_EOF, cur, p, 1);
  return head.next;
}