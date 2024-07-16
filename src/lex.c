#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "sb.h"
#include "utils.h"

bool lex_ident(tokens_t *current, const char *source, size_t *i) {
    current->type = TOKEN_IDENT;

    current->as_ident.sb = sb_new();
    if (current->as_ident.sb == NULL) {
        return false;
    }

    while (isalnum(source[*i]) || source[*i] == '_') {
        sb_addc(current->as_ident.sb, source[*i]);
        (*i)++;
    }

    return true;
}

bool lex_symbol(tokens_t *current, const char *source, size_t *i) {
    if (source[*i] == '(') {
        current->type = TOKEN_LPAR;
        (*i)++;
        return true;
    }
    if (source[*i] == ')') {
        current->type = TOKEN_RPAR;
        (*i)++;
        return true;
    }
    if (source[*i] == '{') {
        current->type = TOKEN_LBRACE;
        (*i)++;
        return true;
    }
    if (source[*i] == '}') {
        current->type = TOKEN_RBRACE;
        (*i)++;
        return true;
    }
    if (source[*i] == ',') {
        current->type = TOKEN_COMMA;
        (*i)++;
        return true;
    }
    if (source[*i] == '+') {
        current->type = TOKEN_PLUS;
        (*i)++;
        return true;
    }
    if (source[*i] == ';') {
        current->type = TOKEN_SEMI;
        (*i)++;
        return true;
    }

    ERROR("unexpected character '%c'", source[*i]);
    return false;
}

bool lex_single(tokens_t *current, const char *source, size_t *i) {
    if (isalpha(source[*i])) {
        return lex_ident(current, source, i);
    }
    return lex_symbol(current, source, i);
}

tokens_t *tokens_new(void) {
    tokens_t *tokens = malloc(sizeof(tokens_t));
    if (tokens == NULL) {
        ERROR("couldn't malloc token", NULL);
        return NULL;
    }

    tokens->next = NULL;
    tokens->type = TOKEN_EOF;

    return tokens;
}

tokens_t *tokens_from_file(const char *path) {
    sb_t *sb = sb_from_file(path);
    if (sb == NULL) {
        return NULL;
    }

    tokens_t *tokens = tokens_new();
    if (tokens == NULL) {
        sb_free(sb);
        return NULL;
    }
    tokens_t *current = tokens;

    size_t i = 0;
    while (sb->string[i] != '\0') {
        while (isspace(sb->string[i])) {
            i++;
        }
        if (sb->string[i] == '\0') {
            break;
        }

        if (!lex_single(current, sb->string, &i)) {
            sb_free(sb);
            tokens_free(tokens);
            return NULL;
        }

        current->next = tokens_new();
        if (current->next == NULL) {
            sb_free(sb);
            tokens_free(tokens);
            return NULL;
        }
        current = current->next;
    }

    sb_free(sb);
    return tokens;
}

void tokens_free(tokens_t *tokens) {
    if (tokens->next) {
        tokens_free(tokens->next);
    }

    switch (tokens->type) {
        case TOKEN_EOF:
        case TOKEN_LPAR:
        case TOKEN_RPAR:
        case TOKEN_LBRACE:
        case TOKEN_RBRACE:
        case TOKEN_COMMA:
        case TOKEN_PLUS:
        case TOKEN_SEMI:
            break;
        case TOKEN_IDENT: {
            sb_free(tokens->as_ident.sb);
        } break;
    }

    free(tokens);
}
