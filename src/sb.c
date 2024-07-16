#include "sb.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"

sb_t *sb_new(void) {
    sb_t *sb = malloc(sizeof(sb_t));
    if (sb == NULL) {
        ERROR("couldn't malloc string builder", NULL);
        return NULL;
    }

    sb->length = 0;
    sb->capacity = 1;

    sb->string = malloc(sizeof(char) * 1);
    if (sb->string == NULL) {
        ERROR("couldn't malloc string for string builder", NULL);
        free(sb);
        return NULL;
    }
    sb->string[0] = '\0';

    return sb;
}

sb_t *sb_from_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ERROR("couldn't open file '%s'", path);
        return NULL;
    }

    sb_t *sb = sb_new();
    if (sb == NULL) {
        fclose(file);
        return NULL;
    }

    static char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        sb_add(sb, buffer);
    }

    fclose(file);
    return sb;
}

void sb_free(sb_t *sb) {
    free(sb->string);
    free(sb);
}

void sb_add(sb_t *sb, const char *string) {
    size_t other_length = strlen(string);

    size_t new_capacity = sb->length + other_length + 1;
    if (new_capacity > sb->capacity) {
        while (new_capacity > sb->capacity) {
            sb->capacity *= 2;
        }

        sb->string = realloc(sb->string, sb->capacity);
    }

    strcpy(sb->string + sb->length, string);
    sb->length += other_length;
}

void sb_addc(sb_t *sb, char c) {
    static char buffer[2] = {0};
    buffer[0] = c;

    sb_add(sb, buffer);
}
