#pragma once

#include <stdbool.h>
#include "parse.h"

typedef struct var_map_t {
    struct var_map_t *next;
    const char *name;
    type_t type;
} var_map_t;

#define STATE_VAR_COUNT 1024

typedef size_t qbe_var_t;

typedef struct state_t {
    struct state_t *parent;
    var_map_t *var_map;
    bool vars[STATE_VAR_COUNT];
    bool is_func_body;
} state_t;

typedef struct {
    type_t return_type;
    type_t value_type;
} type_result_t;

bool check_ast(top_t *ast, state_t **state);

bool state_get(state_t *state, const char *name, type_t *out_type);

qbe_var_t state_alloc_var(state_t *state);
void state_dealloc_var(state_t *state, qbe_var_t var);

bool type_can_add(type_t lhs, type_t rhs, type_t *out_type);
