#include "arguments.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

Arguments args_parse(const int argc, const char **argv) {
  if (argc == 1) {
    return DEFAULT_ARGUMENTS;
  }
  if (argc != 2) {
    return BAD_ARGUMENTS;
  }
  const int port_int = strtoimax(argv[1], NULL, 10);
  if (errno == ERANGE) {
    return BAD_ARGUMENTS;
  }
  const Arguments return_args = {
      .port = port_int,
      .error = false,
  };
  return return_args;
}

void args_print_usage() {
  fprintf(stderr, "Usage: %s [ PORT ]\n", COMMAND_NAME);
}