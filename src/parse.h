#pragma once

#include "lex.h"

typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_ADD,
    EXPR_MULT,
} expr_type_t;

struct expr_t;

typedef struct {
    struct expr_t *lhs;
    struct expr_t *rhs;
} expr_binop_t;

typedef struct {
    tokens_t *var;
} expr_var_t;

typedef struct expr_t {
    expr_type_t type;
    union {
        expr_binop_t as_binop;
        expr_var_t as_var;
    };
} expr_t;

typedef enum {
    STMT_BLOCK,
    STMT_RETURN,
} stmt_type_t;

struct stmt_t;

typedef struct {
    struct stmt_t *statements;
} stmt_block_t;

typedef struct {
    expr_t *value;
} stmt_return_t;

typedef struct stmt_t {
    struct stmt_t *next;
    stmt_type_t type;
    union {
        stmt_block_t as_block;
        stmt_return_t as_return;
    };
} stmt_t;

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
    stmt_t *body;
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
