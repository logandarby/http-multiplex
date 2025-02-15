#include "server.h"

#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"

const int LISTEN_BACKLOG = 64;

Server server_create(const int port) {
  bool error = false;

  const int port_string_size = (int)(ceil(log10(port)) + 1);
  char *port_string = (char *)malloc(sizeof(char) * port_string_size);
  snprintf(port_string, port_string_size, "%d", port);

  // Find appropriate sockets to bind to
  int socketfd = -1;
  struct addrinfo *results = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  const int addr_err =
      getaddrinfo(NULL, port_string, &hints, &results);
  if (addr_err || results == NULL) {
    DZ_ERROR(
        "The specified port %d is not able to bind. Please try again",
        port);
    error = true;
    goto cleanup;
  }
  // Attempt to bind to first available socket
  for (struct addrinfo *current = results; current != NULL;
       current = current->ai_next) {
    const int attempt_socket_fd =
        socket(current->ai_family, current->ai_socktype,
               current->ai_protocol);
    if (attempt_socket_fd < 0) {
      continue;
    }
    {
      const int bind_error = bind(attempt_socket_fd, current->ai_addr,
                                  current->ai_addrlen);
      if (bind_error) {
        continue;
      }
    }
    socketfd = attempt_socket_fd;
    break;
  }

  if (socketfd == -1) {
    DZ_ERRORNO(
        "Could not bind to port %d. Please try another, or verify "
        "that this port isn't already running.",
        port);
    error = true;
    goto cleanup;
  }
  {
    const int listen_error = listen(socketfd, LISTEN_BACKLOG);
    if (listen_error) {
      DZ_ERRORNO(
          "Could not bind to port %d. Please try another, of verify "
          "that this port isn't already running",
          port);
      error = true;
      goto cleanup;
    }
  }

cleanup:
  free(port_string);
  freeaddrinfo(results);
  if (error) {
    exit(EXIT_FAILURE);
  }

  Server return_server = {
      .fd = socketfd,
      .port = port,
  };
  return return_server;
}
