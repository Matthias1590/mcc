#pragma once

#include <stdlib.h>

typedef struct {
    size_t capacity;
    size_t length;
    char *string;
} sb_t;

sb_t *sb_new(void);
sb_t *sb_from_file(const char *path);
void sb_free(sb_t **sb);
void sb_add(sb_t *sb, const char *string);
void sb_addc(sb_t *sb, char c);
