#include <stdio.h>
#include "sb.h"
#include "lex.h"
#include "parse.h"
#include "check.h"

int main(void) {
    tokens_t *tokens = tokens_from_file("test.c");
    if (!tokens) {
        return 1;
    }

    // tokens_t *current = tokens;
    // while (current) {
    //     printf("%d\n", current->type);
    //     current = current->next;
    // }

    top_t *ast = ast_from_tokens(tokens);
    if (ast == NULL) {
        tokens_free(tokens);
        return 1;
    }

    state_t *state;
    if (!check_ast(ast, &state)) {
        ast_free(ast);
        tokens_free(tokens);
        return 1;
    }

    // code generation
    // ...

    ast_free(ast);
    tokens_free(tokens);
    return 0;
}
