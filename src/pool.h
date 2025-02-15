#pragma once

#include <stdbool.h>
#include <sys/select.h>

#define POOL_FD_SETSIZE FD_SETSIZE

enum FdDataType {
  FdDataType_FILE,
  FdDataType_CLIENT,
  FdDataType_SERVER,
  FdDataType_COUNT
};

// For a file descriptor to be read, holds the client fd to send the
// file to
typedef struct {
  int client_fd_to_send;
} FdDataFile;

// Holds any info for use for a client interaction
// The enum data_type determines which union member is to be accessed
typedef struct {
  enum FdDataType data_type;
  union {
    FdDataFile fdDataFile;
  };
} FdData;

typedef struct {
  int socket_fd;
  fd_set read_set;          // FDs to read
  fd_set select_ready_set;  // Buffer for select
  int maxfd;                // Maximum value of fds in read_set
  int nready;  // Number of ready file descriptors from select
  FdData fd_data[POOL_FD_SETSIZE];  // Data for client with i-th fd
} FdPool;

void fdpool_init(FdPool *self, const int socket_fd);

void fdpool_add_fd(FdPool *self, int fd, enum FdDataType);

void fdpool_remove_fd(FdPool *self, int fd);

bool fdpool_contains_fd(FdPool *self, int fd);

bool fdpool_is_fd_ready(FdPool *self, const int fd);

int fdpool_select_ready(FdPool *self, unsigned int timeout);

enum FdDataType fdpool_get_fd_type(FdPool *self, int fd);

void fdpool_set_file_data(FdPool *self, int fd_to_read, FdDataFile);
FdDataFile fdpool_get_file_data(FdPool *self, int fd_to_read);
