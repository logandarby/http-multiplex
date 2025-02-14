#ifndef _ARGUMENTS_H
#define _ARGUMENTS_H

#include <stdbool.h>

typedef struct {
  const int port;
  const bool error;
} Arguments;

static const int DEFAULT_PORT = 8080;
static const char *COMMAND_NAME = "dz_server";

static const Arguments BAD_ARGUMENTS = {
    .port = -1,
    .error = true,
};
static const Arguments DEFAULT_ARGUMENTS = {
    .port = DEFAULT_PORT,
    .error = false,
};

Arguments args_parse(const int argc, const char **argv);
void args_print_usage();

#endif