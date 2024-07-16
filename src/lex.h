#pragma once

#include "sb.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENT,
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_PLUS,
    TOKEN_SEMI,
} token_type_t;

typedef struct {
    sb_t *sb;
} token_ident_t;

typedef struct tokens_t {
    struct tokens_t *next;
    token_type_t type;
    union {
        token_ident_t as_ident;
    };
} tokens_t;

tokens_t *tokens_from_file(const char *path);
void tokens_free(tokens_t *tokens);
