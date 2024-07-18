#include "qbe.h"

#include <stdio.h>
#include "utils.h"

void gen_type(type_t type, state_t *state) {
    (void)state;

    switch (type.type) {
    case TYPE_NONE:
        ERROR("cannot generate type none");
        exit(1);
        break;
    case TYPE_FUNC:
        ERROR("todo: implement");
        exit(1);
        break;
    case TYPE_PRIMITIVE:
        switch (type.as_primitive) {
        case PRIMITIVE_VOID:
            ERROR("cannot generate type void");
            exit(1);
            break;
        case PRIMITIVE_INT:
            printf("w");
            break;
        case PRIMITIVE_BOOL:
            printf("b");
            break;
        }
    }
}

void gen_param(params_t *param, state_t *state) {
    gen_type(param->type, state);
    printf(" %%var_%s", param->name->as_ident.sb->string);
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
    printf("%%temp_%zu =", res);

    type_t res_type;
    type_can_add(add->lhs->cached_type, add->rhs->cached_type, &res_type);
    gen_type(res_type, state);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    printf(" add %%temp_%zu, %%temp_%zu\n", lhs, rhs);

    return res;
}

qbe_var_t gen_mult(expr_binop_t *mult, state_t *state) {
    qbe_var_t lhs = gen_expr(mult->lhs, state);
    qbe_var_t rhs = gen_expr(mult->rhs, state);

    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =", res);

    type_t res_type;
    type_can_add(mult->lhs->cached_type, mult->rhs->cached_type, &res_type);
    gen_type(res_type, state);

    printf(" mul %%temp_%zu, %%temp_%zu\n", lhs, rhs);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    return res;
}

qbe_var_t gen_lt(expr_binop_t *lt, state_t *state) {
    qbe_var_t lhs = gen_expr(lt->lhs, state);
    qbe_var_t rhs = gen_expr(lt->rhs, state);

    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =w ", res);

    // todo: use unsigned comparisons if the types require it
    printf("csltw %%temp_%zu, %%temp_%zu\n", lhs, rhs);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    return res;
}

qbe_var_t gen_gt(expr_binop_t *gt, state_t *state) {
    qbe_var_t lhs = gen_expr(gt->lhs, state);
    qbe_var_t rhs = gen_expr(gt->rhs, state);

    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =w ", res);

    // todo: use unsigned comparisons if the types require it
    printf("csgtw %%temp_%zu, %%temp_%zu\n", lhs, rhs);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    return res;
}

qbe_var_t gen_eq(expr_binop_t *eq, state_t *state) {
    qbe_var_t lhs = gen_expr(eq->lhs, state);
    qbe_var_t rhs = gen_expr(eq->rhs, state);

    // todo: maybe return a single byte instead of an int for comparisons?
    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =w ", res);

    printf("ceqw %%temp_%zu, %%temp_%zu\n", lhs, rhs);

    state_dealloc_var(state, lhs);
    state_dealloc_var(state, rhs);

    return res;
}

qbe_var_t gen_var(expr_t *var, state_t *state) {
    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =", res);

    gen_type(var->cached_type, state);

    printf(" copy %%var_%s\n", var->as_var.var->as_ident.sb->string);

    return res;
}

qbe_var_t gen_int(expr_t *i, state_t *state) {
    qbe_var_t res = state_alloc_var(state);
    printf("%%temp_%zu =", res);

    gen_type(i->cached_type, state);

    printf(" copy %zd\n", i->as_int.token->as_int.value);

    return res;
}

qbe_var_t gen_expr(expr_t *expr, state_t *state) {
    switch (expr->type) {
    case EXPR_ADD:
        return gen_add(&expr->as_binop, state);
    case EXPR_MULT:
        return gen_mult(&expr->as_binop, state);
    case EXPR_LT:
        return gen_lt(&expr->as_binop, state);
    case EXPR_GT:
        return gen_gt(&expr->as_binop, state);
    case EXPR_EQ:
        return gen_eq(&expr->as_binop, state);
    case EXPR_VAR:
        return gen_var(expr, state);
    case EXPR_INT:
        return gen_int(expr, state);
    default:
        ERROR("unimplemented %d", expr->type);
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
    printf("ret %%temp_%zu\n", var);
    state_dealloc_var(state, var);
}

void gen_var_decl(stmt_var_decl_t *var_decl, state_t *state) {
    qbe_var_t expr;
    if (var_decl->value != NULL) {
        expr = gen_expr(var_decl->value, state);
    }

    printf("%%var_%s =", var_decl->name->as_ident.sb->string);
    gen_type(var_decl->type, state);
    printf(" copy ");

    if (var_decl->value != NULL) {
        printf("%%temp_%zu\n", expr);

        state_dealloc_var(state, expr);
    } else {
        printf("0\n");
    }
}

void gen_assign(stmt_assign_t *assign, state_t *state) {
    // todo: writing to a variable that shadows another variable writes to both variables
    // could be fixed by adding the depth of the variable to its name (if its at the global
    // scope and is called x then it would be var_0_x)

    qbe_var_t expr = gen_expr(assign->value, state);

    printf("%%var_%s =", assign->name->as_ident.sb->string);
    gen_type(assign->cached_type, state);
    printf(" copy %%temp_%zu\n", expr);

    state_dealloc_var(state, expr);
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
    case STMT_VAR_DECL:
        gen_var_decl(&stmt->as_var_decl, state);
        break;
    case STMT_ASSIGN:
        gen_assign(&stmt->as_assign, state);
        break;
    default:
        ERROR("unimplemented %d", stmt->type);
        exit(1);
    }
}

void gen_func_decl(top_func_decl_t *func_decl, state_t *state) {
    printf("export function ");
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
    printf("}\n");
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
}
