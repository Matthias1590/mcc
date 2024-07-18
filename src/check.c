#include "check.h"

#include <string.h>
#include "utils.h"

state_t *state_new(void) {
    state_t *state = malloc(sizeof(state_t));
    if (state == NULL) {
        ERROR("couldn't malloc state");
        return NULL;
    }

    state->parent = NULL;
    state->var_map = NULL;

    for (size_t i = 0; i < STATE_VAR_COUNT; i++) {
        state->vars[i] = false;
    }

    return state;
}

state_t *state_new_substate(state_t *parent) {
    state_t *substate = state_new();

    if (substate != NULL) {
        substate->parent = parent;
    }

    return substate;
}

var_map_t *var_map_new(void) {
    var_map_t *var_map = malloc(sizeof(var_map_t));
    if (var_map == NULL) {
        ERROR("couldn't malloc variable map");
        return NULL;
    }

    var_map->next = NULL;

    return var_map;
}

bool state_add(state_t *state, const char *name, type_t type) {
    var_map_t *current = state->var_map;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            ERROR("a variable with the name '%s' already exists in the current scope", name);
            return false;
        }

        current = current->next;
    }

    var_map_t *var_map;
    if (state->var_map == NULL) {
        state->var_map = var_map_new();
        if (state->var_map == NULL) {
            return false;
        }
        var_map = state->var_map;
    } else {
        var_map = state->var_map;
        while (var_map->next != NULL) {
            var_map = var_map->next;
        }
        var_map->next = var_map_new();
        if (var_map->next == NULL) {
            return false;
        }
        var_map = var_map->next;
    }

    var_map->name = name;
    var_map->type = type;

    return true;
}

bool state_get(state_t *state, const char *name, type_t *out_type) {
    var_map_t *current = state->var_map;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            *out_type = current->type;
            return true;
        }
        current = current->next;
    }

    if (state->parent != NULL) {
        return state_get(state->parent, name, out_type);
    }

    return false;
}

qbe_var_t state_alloc_var(state_t *state) {
    if (state->parent != NULL) {
        return state_alloc_var(state->parent);
    }

    for (qbe_var_t var = 0; var < STATE_VAR_COUNT; var++) {
        if (!state->vars[var]) {
            state->vars[var] = true;
            return var;
        }
    }

    ERROR("ran out of temporary variables while generating code");
    exit(1);
}

void state_dealloc_var(state_t *state, qbe_var_t var) {
    if (state->parent != NULL) {
        state_dealloc_var(state->parent, var);
        return;
    }

    state->vars[var] = false;
}

bool type_can_be_converted_to(type_t from, type_t to) {
    if (from.type != to.type) {
        return false;
    }

    switch (from.type) {
    case TYPE_NONE:
        return false;
    case TYPE_PRIMITIVE:
        // todo: add promotion stuff, type_can_be_converted_to(int, long) should be true
        return from.as_primitive == to.as_primitive;
    case TYPE_FUNC:
        // todo: implement this
        return false;
    }
}

bool type_can_add(type_t lhs, type_t rhs, type_t *out_type) {
    if (lhs.type != rhs.type) {
        return false;
    }

    switch (lhs.type) {
    case TYPE_NONE:
        return false;
    case TYPE_PRIMITIVE:
        // todo: add promotion, same story as in type_can_be_converted_to
        switch (lhs.as_primitive) {
        case PRIMITIVE_VOID:
            return false;
        case PRIMITIVE_INT:
            if (out_type != NULL) {
                out_type->type = TYPE_PRIMITIVE;
                out_type->as_primitive = PRIMITIVE_INT;
            }
            return rhs.as_primitive == PRIMITIVE_INT;
        }
    case TYPE_FUNC:
        // todo: can you add functions?
        return false;
    }
}

bool type_can_mult(type_t lhs, type_t rhs, type_t *out_type) {
    if (lhs.type != rhs.type) {
        return false;
    }

    switch (lhs.type) {
    case TYPE_NONE:
        return false;
    case TYPE_PRIMITIVE:
        // todo: add promotion, same story as in type_can_be_converted_to
        switch (lhs.as_primitive) {
        case PRIMITIVE_VOID:
            return false;
        case PRIMITIVE_INT:
            if (out_type != NULL) {
                out_type->type = TYPE_PRIMITIVE;
                out_type->as_primitive = PRIMITIVE_INT;
            }
            return rhs.as_primitive == PRIMITIVE_INT;
        }
    case TYPE_FUNC:
        // todo: can you multiply functions?
        return false;
    }
}

bool check_stmt(state_t *state, stmt_t *stmt, type_result_t *type_result);

bool check_block(state_t *state, stmt_block_t block, type_result_t *type_result) {
    stmt_t *stmt = block.statements;

    while (stmt != NULL && stmt->type != STMT_NONE) {
        if (!check_stmt(state, stmt, type_result)) {
            return false;
        }
        stmt = stmt->next;
    }

    return true;
}

bool check_expr(state_t *state, expr_t *expr, type_result_t *type_result);

bool check_add(state_t *state, expr_binop_t *binop, type_result_t *type_result) {
    if (!check_expr(state, binop->lhs, type_result)) {
        return false;
    }
    type_t lhs_type = type_result->value_type;

    if (!check_expr(state, binop->rhs, type_result)) {
        return false;
    }
    type_t rhs_type = type_result->value_type;

    if (!type_can_add(lhs_type, rhs_type, &type_result->value_type)) {
        return false;
    }

    return true;
}

bool check_mult(state_t *state, expr_binop_t *binop, type_result_t *type_result) {
    if (!check_expr(state, binop->lhs, type_result)) {
        return false;
    }
    type_t lhs_type = type_result->value_type;

    if (!check_expr(state, binop->rhs, type_result)) {
        return false;
    }
    type_t rhs_type = type_result->value_type;

    if (!type_can_mult(lhs_type, rhs_type, &type_result->value_type)) {
        return false;
    }

    return true;
}

bool check_var(state_t *state, expr_var_t *var, type_result_t *type_result) {
    return state_get(state, var->var->as_ident.sb->string, &type_result->value_type);
}

bool check_expr(state_t *state, expr_t *expr, type_result_t *type_result) {
    bool res = false;
    switch (expr->type) {
    case EXPR_ADD:
        res = check_add(state, &expr->as_binop, type_result);
        break;
    case EXPR_MULT:
        res = check_mult(state, &expr->as_binop, type_result);
        break;
    case EXPR_VAR:
        res = check_var(state, &expr->as_var, type_result);
        break;
    case EXPR_INT:
        return true;  // todo: maybe check int bounds? type_result.value_type could be set to the smallest int type needed to contain the literal
    }

    expr->cached_type = type_result->value_type;
    return res;
}

bool check_return(state_t *state, stmt_return_t ret, type_result_t *type_result) {
    type_result->return_type.type = TYPE_PRIMITIVE;
    type_result->return_type.as_primitive = PRIMITIVE_VOID;
    if (ret.value != NULL) {
        if (!check_expr(state, ret.value, type_result)) {
            return false;
        }
        type_result->return_type = type_result->value_type;
    }

    return true;
}

bool check_stmt(state_t *state, stmt_t *stmt, type_result_t *type_result) {
    switch (stmt->type) {
    case STMT_NONE:
        return true;
    case STMT_BLOCK: {
        state_t *substate = state_new_substate(state);
        return check_block(substate, stmt->as_block, type_result);
    } break;
    case STMT_RETURN:
        return check_return(state, stmt->as_return, type_result);
    }
}

bool check_func_decl(state_t *state, top_func_decl_t *func_decl) {
    state_add(state, func_decl->name->as_ident.sb->string, (type_t) {
        .type = TYPE_FUNC,
        .as_func = (type_func_t) {
            .return_type = &func_decl->return_type,
            .params = func_decl->params,
        }
    });

    state_t *substate = state_new_substate(state);

    params_t *params = func_decl->params;
    while (params != NULL && params->name != NULL) {
        state_add(substate, params->name->as_ident.sb->string, params->type);
        params = params->next;
    }

    type_result_t type_result;
    if (!check_stmt(substate, func_decl->body, &type_result)) {
        return false;
    }

    if (!type_can_be_converted_to(func_decl->return_type, type_result.return_type)) {
        return false;
    }

    // todo: free the substate
    return true;
}

bool check_top(state_t *state, top_t *top) {
    switch (top->type) {
    case TOP_NONE:
        return true;
    case TOP_FUNC_DECL:
        return check_func_decl(state, &top->as_func_decl);
    }
}

bool check_ast(top_t *ast, state_t **state) {
    *state = state_new();
    if (state == NULL) {
        return false;
    }

    while (ast->type != TOP_NONE) {
        if (!check_top(*state, ast)) {
            return false;
        }

        ast = ast->next;
    }

    return true;
}
