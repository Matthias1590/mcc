#include "qbe.h"

#include <stdio.h>
#include "utils.h"

void gen_type(type_t type, state_t *state) {
    (void)state;

    switch (type.type) {
    case TYPE_FUNC:
        ERROR("todo: implement");
        exit(1);
    case TYPE_PRIMITIVE:
        switch (type.as_primitive) {
        case PRIMITIVE_VOID:
            break;
        case PRIMITIVE_INT:
            printf("w");
        }
    }
}

void gen_param(params_t *param, state_t *state) {
    gen_type(param->type, state);
    printf(" %%%s", param->name->as_ident.sb->string);
}

void gen_stmt(stmt_t *stmt, state_t *state);

void gen_block(stmt_block_t *block, state_t *state) {
    stmt_t *current = block->statements;
    while (current != NULL && current->type != STMT_NONE) {
        gen_stmt(current, state);

        current = current->next;
    }
}

qbe_var_t gen_expr(expr_t *expr, state_t *state);

qbe_var_t gen_add(expr_binop_t *add, state_t *state) {
    qbe_var_t lhs = gen_expr(add->lhs, state);
    qbe_var_t rhs = gen_expr(add->rhs, state);

    qbe_var_t res = state_alloc_var(state);
    printf("%%%zu =", res);

    type_t res_type;
    type_can_add(add->lhs->cached_type, add->rhs->cached_type, &res_type);
    gen_type(res_type, state);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    printf(" add %%%zu, %%%zu\n", lhs, rhs);

    return res;
}

qbe_var_t gen_mult(expr_binop_t *mult, state_t *state) {
    qbe_var_t lhs = gen_expr(mult->lhs, state);
    qbe_var_t rhs = gen_expr(mult->rhs, state);

    qbe_var_t res = state_alloc_var(state);
    printf("%%%zu =", res);

    type_t res_type;
    type_can_add(mult->lhs->cached_type, mult->rhs->cached_type, &res_type);
    gen_type(res_type, state);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    printf(" mult %%%zu, %%%zu\n", lhs, rhs);

    return res;
}

qbe_var_t gen_var(expr_var_t *var, state_t *state) {
    qbe_var_t res = state_alloc_var(state);
    printf("%%%zu =", res);

    type_t res_type;
    state_get(state, var->var->as_ident.sb->string, &res_type);
    gen_type(res_type, state);

    printf(" copy %%%s\n", var->var->as_ident.sb->string);

    return res;
}

qbe_var_t gen_expr(expr_t *expr, state_t *state) {
    switch (expr->type) {
    case EXPR_ADD:
        return gen_add(&expr->as_binop, state);
    case EXPR_MULT:
        return gen_mult(&expr->as_binop, state);
    case EXPR_VAR:
        return gen_var(&expr->as_var, state);
    case EXPR_INT:
        ERROR("todo: implement");
        exit(1);
        return 0;
    }
}

void gen_return(stmt_return_t *ret, state_t *state) {
    if (ret->value == NULL) {
        printf("ret\n");
        return;
    }

    qbe_var_t var = gen_expr(ret->value, state);
    printf("ret %%%zu\n", var);
    state_dealloc_var(state, var);
}

void gen_stmt(stmt_t *stmt, state_t *state) {
    switch (stmt->type) {
    case STMT_NONE:
        break;
    case STMT_BLOCK:
        gen_block(&stmt->as_block, state);
        break;
    case STMT_RETURN:
        gen_return(&stmt->as_return, state);
        break;
    }
}

void gen_func_decl(top_func_decl_t *func_decl, state_t *state) {
    printf("function ");
    gen_type(func_decl->return_type, state);
    printf(" $%s(", func_decl->name->as_ident.sb->string);

    bool printed_param = false;
    params_t *params = func_decl->params;
    while (params != NULL && params->name != NULL) {
        if (printed_param) {
            printf(", ");
        }
        printed_param = true;
        gen_param(params, state);
        params = params->next;
    }
    printf(") {\n@start\n");
    gen_stmt(func_decl->body, state);
    printf("}");
}

void gen_top(top_t *ast, state_t *state) {
    switch (ast->type) {
    case TOP_NONE:
        break;
    case TOP_FUNC_DECL:
        gen_func_decl(&ast->as_func_decl, state);
        break;
    }
}

void gen_code(top_t *ast, state_t *state) {
    while (ast != NULL && ast->type != TOP_NONE) {
        gen_top(ast, state);

        ast = ast->next;
    }
    printf("\n");
}
