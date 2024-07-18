#include "parse.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"

bool parse_stmt(stmt_t *stmt, tokens_t **tokens);

top_t *ast_new(void) {
    top_t *ast = malloc(sizeof(top_t));
    if (ast == NULL) {
        ERROR("couldn't malloc ast");
        return NULL;
    }

    ast->next = NULL;
    ast->type = TOP_NONE;

    return ast;
}

void ast_free(top_t *ast) {
    // todo: switch on type

    // free(ast);
    (void)ast;
}

stmt_t *stmt_new(void) {
    stmt_t *stmt = malloc(sizeof(stmt_t));
    if (stmt == NULL) {
        ERROR("couldn't malloc statement");
        return NULL;
    }

    stmt->next = NULL;
    stmt->type = STMT_NONE;

    return stmt;
}

void stmt_free(stmt_t *stmt) {
    // todo: switch on type

    if (stmt->next) {
        stmt_free(stmt->next);
    }

    // free(stmt);
}

expr_t *expr_new(void) {
    expr_t *expr = malloc(sizeof(expr_t));
    if (expr == NULL) {
        ERROR("couldn't malloc expression");
        return NULL;
    }

    expr->cached_type.type = TYPE_NONE;

    return expr;
}

void expr_free(expr_t *expr) {
    // todo: switch on type

    // free(expr);
    (void)expr;
}

params_t *params_new(void) {
    params_t *params = malloc(sizeof(params_t));
    if (params == NULL) {
        ERROR("couldn't malloc params");
        return NULL;
    }

    params->next = NULL;
    params->name = NULL;

    return params;
}

void params_free(params_t *params) {
    if (params->next) {
        params_free(params->next);
    }

    // free(params);
}

bool parse_token(token_type_t type, tokens_t **token, tokens_t **tokens) {
    if ((*tokens)->type != type) {
        return false;
    }

    if (token != NULL) {
        *token = *tokens;
    }
    *tokens = (*tokens)->next;

    return true;
}

bool parse_type(type_t *type, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    tokens_t *token;
    if (!parse_token(TOKEN_IDENT, &token, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (strcmp(token->as_ident.sb->string, "int") == 0) {
        type->type = TYPE_PRIMITIVE;
        type->as_primitive = PRIMITIVE_INT;
        return true;
    }
    if (strcmp(token->as_ident.sb->string, "void") == 0) {
        type->type = TYPE_PRIMITIVE;
        type->as_primitive = PRIMITIVE_VOID;
        return true;
    }
    if (strcmp(token->as_ident.sb->string, "_Bool") == 0) {
        type->type = TYPE_PRIMITIVE;
        type->as_primitive = PRIMITIVE_BOOL;
        return true;
    }

    // i just realized, how am i supposed to parse array types, pointers even...
    // theyre part of the variable name, not the type...
    // todo: figure this out later ^

    *tokens = start_token;
    return false;
}

bool parse_param(params_t *params, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    type_t type;
    if (!parse_type(&type, tokens)) {
        *tokens = start_token;
        return false;
    }

    tokens_t *name;
    if (!parse_token(TOKEN_IDENT, &name, tokens)) {
        *tokens = start_token;
        return false;
    }

    params->type = type;
    params->name = name;

    return true;
}

bool parse_params(params_t *params, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    bool parsed_param = false;
    while ((*tokens)->type != TOKEN_RPAR) {
        if (parsed_param) {
            if (!parse_token(TOKEN_COMMA, NULL, tokens)) {
                *tokens = start_token;
                return false;
            }
        }

        if (!parse_param(params, tokens)) {
            *tokens = start_token;
            return false;
        }

        parsed_param = true;
        params->next = params_new();
        params = params->next;
    }

    return true;
}

bool parse_term(expr_t **expr, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    tokens_t *token;
    
    if (parse_token(TOKEN_IDENT, &token, tokens)) {
        *expr = expr_new();
        (*expr)->type = EXPR_VAR;
        (*expr)->as_var.var = token;
        return true;
    }

    if (parse_token(TOKEN_INT, &token, tokens)) {
        *expr = expr_new();
        (*expr)->type = EXPR_INT;
        (*expr)->as_int.token = token;
        return true;
    }

    *tokens = start_token;
    return false;
}

bool parse_compare_op(expr_t **expr, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    // try to parse multiplications, lhs = parse_term(), rhs = parse_term()
    expr_t *lhs;
    if (!parse_term(&lhs, tokens)) {
        *tokens = start_token;
        return false;
    }

    while (true) {
        expr_type_t expr_type;

        tokens_t *op_token = NULL;
        if (parse_token(TOKEN_LT, &op_token, tokens)) {
            expr_type = EXPR_LT;
        } else if (parse_token(TOKEN_GT, &op_token, tokens)) {
            expr_type = EXPR_GT;
        } else if (parse_token(TOKEN_EQ, &op_token, tokens)) {
            expr_type = EXPR_EQ;
        }

        if (op_token == NULL) {
            break;
        }

        expr_t *rhs;
        if (!parse_term(&rhs, tokens)) {
            expr_free(lhs);
            *tokens = start_token;
            return false;
        }

        expr_t *temp = expr_new();
        temp->type = expr_type;
        temp->as_binop.lhs = lhs;
        temp->as_binop.rhs = rhs;
        lhs = temp;
    }

    *expr = lhs;
    return true;
}

bool parse_mult_op(expr_t **expr, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    // try to parse multiplications, lhs = parse_compare_op(), rhs = parse_compare_op()
    expr_t *lhs;
    if (!parse_compare_op(&lhs, tokens)) {
        *tokens = start_token;
        return false;
    }

    while (true) {
        tokens_t *op_token = NULL;
        parse_token(TOKEN_STAR, &op_token, tokens);
        // || parse_token(TOKEN_MINUS, &op_token, tokens);

        if (op_token == NULL) {
            break;
        }

        expr_t *rhs;
        if (!parse_compare_op(&rhs, tokens)) {
            expr_free(lhs);
            *tokens = start_token;
            return false;
        }

        expr_t *temp = expr_new();
        temp->type = EXPR_MULT;  // todo: switch on op_token->type, dont always assume its a multiplication
        temp->as_binop.lhs = lhs;
        temp->as_binop.rhs = rhs;
        lhs = temp;
    }

    *expr = lhs;
    return true;
}

bool parse_add_op(expr_t **expr, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    // try to parse additions, lhs = parse_mult_op(), rhs = parse_mult_op()
    expr_t *lhs;
    if (!parse_mult_op(&lhs, tokens)) {
        *tokens = start_token;
        return false;
    }

    while (true) {
        tokens_t *op_token = NULL;
        parse_token(TOKEN_PLUS, &op_token, tokens);
        // || parse_token(TOKEN_MINUS, &op_token, tokens);

        if (op_token == NULL) {
            break;
        }

        expr_t *rhs;
        if (!parse_mult_op(&rhs, tokens)) {
            expr_free(lhs);
            *tokens = start_token;
            return false;
        }

        expr_t *temp = expr_new();
        temp->type = EXPR_ADD;  // todo: switch on op_token->type, dont always assume its an addition
        temp->as_binop.lhs = lhs;
        temp->as_binop.rhs = rhs;
        lhs = temp;
    }

    *expr = lhs;
    return true;
}

bool parse_expr(expr_t **expr, tokens_t **tokens) {
    return parse_add_op(expr, tokens);
}

bool parse_block(stmt_t *stmt, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    if (!parse_token(TOKEN_LBRACE, NULL, tokens)) {
        *tokens = start_token;
        return false;
    }

    stmt->type = STMT_BLOCK;
    stmt->as_block.statements = stmt_new();

    stmt_t *inner = stmt->as_block.statements;
    while ((*tokens)->type != TOKEN_RBRACE) {
        if (!parse_stmt(inner, tokens)) {
            stmt_free(stmt->as_block.statements);
            *tokens = start_token;
            return false;
        }

        inner->next = stmt_new();
        inner = inner->next;
    }

    if (!parse_token(TOKEN_RBRACE, NULL, tokens)) {
        stmt_free(stmt->as_block.statements);
        *tokens = start_token;
        return false;
    }

    return true;
}

bool parse_var_decl(stmt_t *stmt, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    stmt->type = STMT_VAR_DECL;

    if (!parse_type(&stmt->as_var_decl.type, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_IDENT, &stmt->as_var_decl.name, tokens)) {
        *tokens = start_token;
        return false;
    }

    stmt->as_var_decl.value = NULL;

    if (parse_token(TOKEN_ASSIGN, NULL, tokens)) {
        if (!parse_expr(&stmt->as_var_decl.value, tokens)) {
            expr_free(stmt->as_var_decl.value);
            *tokens = start_token;
            return false;
        }
    }

    if (!parse_token(TOKEN_SEMI, NULL, tokens)) {
        if (stmt->as_var_decl.value != NULL) {
            expr_free(stmt->as_var_decl.value);
        }
        *tokens = start_token;
        return false;
    }

    return true;
}

bool parse_assign(stmt_t *stmt, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    stmt->type = STMT_ASSIGN;

    if (!parse_token(TOKEN_IDENT, &stmt->as_assign.name, tokens)) {
        *tokens = start_token;
        return false;
    }

    stmt->as_assign.value = NULL;

    if (!parse_token(TOKEN_ASSIGN, NULL, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (!parse_expr(&stmt->as_assign.value, tokens)) {
        expr_free(stmt->as_assign.value);
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_SEMI, NULL, tokens)) {
        expr_free(stmt->as_assign.value);
        *tokens = start_token;
        return false;
    }

    return true;
}

bool parse_return(stmt_t *stmt, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    if (!parse_token(TOKEN_KW_RETURN, NULL, tokens)) {
        *tokens = start_token;
        return false;
    }

    stmt->type = STMT_RETURN;

    if (!parse_expr(&stmt->as_return.value, tokens)) {
        expr_free(stmt->as_return.value);
        stmt->as_return.value = NULL;
    }

    if (!parse_token(TOKEN_SEMI, NULL, tokens)) {
        if (stmt->as_return.value != NULL) {
            expr_free(stmt->as_return.value);
            stmt->as_return.value = NULL;
        }
        *tokens = start_token;
        return false;
    }

    return true;
}

bool parse_stmt(stmt_t *stmt, tokens_t **tokens) {
    if (parse_block(stmt, tokens)) {
        return true;
    }
    if (parse_var_decl(stmt, tokens)) {
        return true;
    }
    if (parse_assign(stmt, tokens)) {
        return true;
    }
    if (parse_return(stmt, tokens)) {
        return true;
    }
    return false;
}

bool parse_func_decl(top_t *current, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    current->type = TOP_FUNC_DECL;

    if (!parse_type(&current->as_func_decl.return_type, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_IDENT, &current->as_func_decl.name, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_LPAR, NULL, tokens)) {
        *tokens = start_token;
        return false;
    }

    current->as_func_decl.params = params_new();
    if (!parse_params(current->as_func_decl.params, tokens)) {
        params_free(current->as_func_decl.params);
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_RPAR, NULL, tokens)) {
        params_free(current->as_func_decl.params);
        *tokens = start_token;
        return false;
    }

    current->as_func_decl.body = stmt_new();
    if (!parse_block(current->as_func_decl.body, tokens)) {
        params_free(current->as_func_decl.params);
        stmt_free(current->as_func_decl.body);
        *tokens = start_token;
        return false;
    }

    return true;
}

bool parse_top(top_t *current, tokens_t **tokens) {
    // todo: find out what path went the furthest (token-wise) and then show its error.
    // if we almost parsed a full function but the last statement was wrong, we should show
    // the error of the statement parsing, not "expected function declaration"

    tokens_t *start_token = *tokens;

    if (parse_func_decl(current, tokens)) {
        return true;
    }

    *tokens = start_token;
    return false;
}

top_t *ast_from_tokens(tokens_t *tokens) {
    top_t *ast = ast_new();
    if (ast == NULL) {
        return NULL;
    }
    top_t *current = ast;

    while (tokens->type != TOKEN_EOF) {
        if (!parse_top(current, &tokens)) {
            ast_free(ast);
            return NULL;
        }

        current->next = ast_new();
        if (current->next == NULL) {
            ast_free(ast);
            return NULL;
        }
        current = current->next;
    }

    return ast;
}

const char *type_to_string(type_t type) {
    switch (type.type) {
    case TYPE_NONE:
        return "none";
    case TYPE_FUNC:
        return "function";
    case TYPE_PRIMITIVE:
        switch (type.as_primitive) {
        case PRIMITIVE_INT:
            return "int";
        case PRIMITIVE_VOID:
            return "void";
        case PRIMITIVE_BOOL:
            return "bool";
        }
    }
}
