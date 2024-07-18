#include <stdio.h>
#include "sb.h"
#include "lex.h"
#include "parse.h"

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

    ast_free(ast);
    tokens_free(tokens);
    return 0;
}
