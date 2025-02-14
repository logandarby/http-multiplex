#include "core.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void exit_error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

bool str_eq(const char *s1, const char *s2, size_t n) {
  return strncmp(s1, s2, n) == 0;
}
