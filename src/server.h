#pragma once

#include "core.h"

typedef struct {
  int fd;    // File descriptor for the server object
  int port;  // Port of the server that is listening
} Server;

extern const int LISTEN_BACKLOG;

// Attemps to bind a socket to the specified port, and starts
// listeting. Returns a server object with the appripriate information
extern Server server_create(const int port);
