#include "parse.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"

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
    free(ast);
}

stmt_t *stmt_new(void) {
    stmt_t *stmt = malloc(sizeof(stmt_t));
    if (stmt == NULL) {
        ERROR("couldn't malloc statement");
        return NULL;
    }

    stmt->next = NULL;

    return stmt;
}

void stmt_free(stmt_t *stmt) {
    if (stmt->next) {
        stmt_free(stmt->next);
    }

    free(stmt);
}

params_t *params_new(void) {
    params_t *params = malloc(sizeof(params_t));
    if (params == NULL) {
        ERROR("couldn't malloc params");
        return NULL;
    }

    params->next = NULL;

    return params;
}

void params_free(params_t *params) {
    if (params->next) {
        params_free(params->next);
    }

    free(params);
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

    while ((*tokens)->type != TOKEN_RPAR) {
        if (!parse_param(params, tokens)) {
            *tokens = start_token;
            return false;
        }

        params->next = params_new();
        params = params->next;
    }

    return true;
}

bool parse_func_decl(top_t *current, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    type_t return_type;
    if (!parse_type(&return_type, tokens)) {
        *tokens = start_token;
        return false;
    }

    tokens_t *name;
    if (!parse_token(TOKEN_IDENT, &name, tokens)) {
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_LPAR, NULL, tokens)) {
        *tokens = start_token;
        return false;
    }

    params_t *params = params_new();
    if (!parse_params(params, tokens)) {
        params_free(params);
        *tokens = start_token;
        return false;
    }

    if (!parse_token(TOKEN_RPAR, NULL, tokens)) {
        params_free(params);
        *tokens = start_token;
        return false;
    }

    stmt_t *body = stmt_new();
    if (!parse_block(body, tokens)) {
        params_free(params);
        stmt_free(body);
        *tokens = start_token;
        return false;
    }

    current->type = TOP_FUNC_DECL;
    current->as_func_decl.return_type = return_type;
    current->as_func_decl.name = name;
    current->as_func_decl.params = params;
    current->as_func_decl.body = body;

    return true;
}

bool parse_top(top_t *current, tokens_t **tokens) {
    tokens_t *start_token = *tokens;

    if (parse_func_decl(current, tokens)) {
        return true;
    }

    *tokens = start_token;
    ERROR("expected a function declaration");
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
