#pragma once

#include <poll.h>
#include <stdlib.h>
#include <sys/socket.h>

// A module for doing file IO, as well as server sending/recveiving.
// Used for dependency injection when testing

typedef struct FileModule {
  int (*open)(const char *filename, int flags, ...);
  int (*close)(int file_descriptor);
  ssize_t (*read)(int file_descriptor, void *buffer,
                  size_t buffer_size);
  ssize_t (*write)(int file_descriptor, const void *buffer,
                   size_t buffer_size);
  int (*select)(int n_fds, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout);
  int (*accept)(int file_descriptor, __SOCKADDR_ARG addr,
                socklen_t *addr_len);
  ssize_t (*recv)(int fd, void *buf, size_t n, int flags);
  ssize_t (*send)(int fd, const void *buf, size_t n, int flags);
  int (*poll)(struct pollfd *fds, nfds_t nfds, int timeout);
} FileModule;

// Light wrappers for the system default IO operations
extern const FileModule SYS_FILE_MODULE;
