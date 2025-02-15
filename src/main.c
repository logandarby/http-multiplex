#include <stdio.h>
#include <stdlib.h>

#include "arguments.h"
#include "interrupt.h"
#include "start.h"

static const unsigned int DEFAULT_TIMEOUT_MS = 10000;

int main(const int argc, const char **argv) {
  const Arguments args = args_parse(argc, argv);
  if (args.error) {
    args_print_usage();
    exit(EXIT_FAILURE);
  }
#ifdef _DEBUG
  printf("Debug signal handling mode enabled\n");
  signal(SIGINT, interrupt_signal_handler);
#endif
  return start(args.port, RESOURCES_PATH, &is_running,
               DEFAULT_TIMEOUT_MS);
}
