#pragma once

#include <sys/types.h>
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
    TOKEN_STAR,
    TOKEN_SEMI,
    TOKEN_KW_RETURN,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQ,
    TOKEN_ASSIGN,
    TOKEN_INT,
} token_type_t;

typedef struct {
    sb_t *sb;
} token_ident_t;

typedef struct {
    ssize_t value;
} token_int_t;

typedef struct tokens_t {
    struct tokens_t *next;
    size_t index;
    const char *file;
    size_t line;
    size_t column;
    token_type_t type;
    union {
        token_ident_t as_ident;
        token_int_t as_int;
    };
} tokens_t;

tokens_t *tokens_from_file(const char *path);
void tokens_free(tokens_t *tokens);

char *token_location(tokens_t *token);
