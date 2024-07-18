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

bool parse_token(token_type_t type, tokens_t **token, tokens_t **tokens) {
    if ((*tokens)->type != type) {
        return false;
    }

    *token = *tokens;
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

    *tokens = start_token;
    return false;
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

    ERROR("todo: parse parameters");
    
    current->type = TOP_FUNC_DECL;
    current->as_func_decl.return_type = return_type;
    current->as_func_decl.name = name;

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
