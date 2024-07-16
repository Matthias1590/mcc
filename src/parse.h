#pragma once

#include "lex.h"

typedef enum {
    TYPE_PRIMITIVE,
} type_type_t;

typedef enum {
    PRIMITIVE_INT,
} type_primitive_t;

typedef struct {
    type_type_t type;
    union {
        type_primitive_t as_primitive;
    };
} type_t;

typedef struct params_t {
    struct params_t *next;
    type_t type;
    tokens_t *name;
} params_t;

typedef struct {
    type_t return_type;
    tokens_t *name;
    params_t *params;
} top_func_decl_t;

typedef enum {
    TOP_NONE,
    TOP_FUNC_DECL,
} top_type_t;

typedef struct top_t {
    top_type_t type;
    struct top_t *next;
    union {
        top_func_decl_t as_func_decl;
    };
} top_t;

top_t *ast_from_tokens(tokens_t *tokens);
void ast_free(top_t *ast);
