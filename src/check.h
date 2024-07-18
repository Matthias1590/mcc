#pragma once

#include <stdbool.h>
#include "parse.h"

typedef struct var_map_t {
    struct var_map_t *next;
    const char *name;
    type_t type;
} var_map_t;

typedef struct state_t {
    struct state_t *parent;
    var_map_t *var_map;
} state_t;

typedef struct {
    type_t return_type;
    type_t value_type;
} type_result_t;

bool check_ast(top_t *ast, state_t **state);
