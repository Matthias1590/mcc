#pragma once

#include "lex.h"

typedef enum {
    TYPE_NONE,
    TYPE_PRIMITIVE,
    TYPE_FUNC,
} type_type_t;

typedef enum {
    PRIMITIVE_INT,
    PRIMITIVE_VOID,
} type_primitive_t;

typedef struct {
    struct type_t *return_type;
    struct params_t *params;
} type_func_t;

typedef struct type_t {
    type_type_t type;
    union {
        type_primitive_t as_primitive;
        type_func_t as_func;
    };
} type_t;

typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_ADD,
    EXPR_MULT,
} expr_type_t;

struct expr_t;
struct params_t;

typedef struct {
    struct expr_t *lhs;
    struct expr_t *rhs;
} expr_binop_t;

typedef struct {
    tokens_t *var;
} expr_var_t;

typedef struct expr_t {
    expr_type_t type;
    type_t cached_type;
    union {
        expr_binop_t as_binop;
        expr_var_t as_var;
    };
} expr_t;

typedef enum {
    STMT_NONE,
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
