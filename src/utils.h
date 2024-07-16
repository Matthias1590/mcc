#pragma once

#include <stdio.h>
#include <stdarg.h>

#define ERROR(message, ...) \
    fprintf(stderr, "[error] %s:%d: "message"\n", __FILE__, __LINE__, ##__VA_ARGS__);
