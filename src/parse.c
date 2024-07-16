#include "parse.h"

#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

top_t *ast_new(void) {
    top_t *ast = malloc(sizeof(top_t));
    if (ast == NULL) {
        ERROR("couldn't malloc ast", NULL);
        return NULL;
    }

    ast->next = NULL;
    ast->type = TOP_NONE;

    return ast;
}

bool parse_single(top_t *current, tokens_t *tokens) {
    (void)current;
    (void)tokens;
    ERROR("todo: implement parsing", NULL);
    return false;
}

top_t *ast_from_tokens(tokens_t *tokens) {
    top_t *ast = ast_new();
    if (ast == NULL) {
        return NULL;
    }
    top_t *current = ast;

    while (tokens->type != TOKEN_EOF) {
        if (!parse_single(current, tokens)) {
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

void ast_free(top_t *ast) {
    free(ast);
}
