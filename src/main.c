#include <stdio.h>
#include "sb.h"
#include "lex.h"

int main(void) {
    tokens_t *tokens = tokens_from_file("test.c");
    if (!tokens) {
        return 1;
    }

    tokens_t *current = tokens;
    while (current) {
        printf("%d\n", current->type);
        current = current->next;
    }

    tokens_free(tokens);

    return 0;
}
