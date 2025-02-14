#pragma once

#include <stdbool.h>
#include <string.h>

#define max(a, b) (a > b ? a : b)
#define array_len(a) (sizeof(a) / sizeof(a[0]))

// Prints an formatted error message and exits
extern void exit_error(const char *msg, ...);

bool str_eq(const char *s1, const char *s2, size_t n);
