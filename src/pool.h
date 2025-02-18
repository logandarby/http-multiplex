#pragma once

#include <stdbool.h>
#include <string.h>

#include "dz_hashmap.h"
#include "file_module.h"

enum FdDataType {
  FdDataType_FILE,
  FdDataType_CLIENT,
  FdDataType_SERVER,
  FdDataType_COUNT
};

// For a file descriptor to be read, holds the client fd to send the
// file to
typedef struct FdDataFile {
  int client_fd_to_send;
} FdDataFile;

// Holds any info for use for a client interaction
// The enum data_type determines which union member is to be accessed
typedef struct FdData {
  enum FdDataType data_type;  // Data type of the fd
  union {                     // Optional Info to supply with the fd
    FdDataFile fd_data_file;
  };
} FdData;

typedef struct FdPool {
  int socket_fd;
  struct pollfd *poll_fd_array;  // Each index refers to its
                                 // corresponding fd
  size_t max_fd;                 // Max fd currently stored
  size_t fd_limit;               // Hard limit on number of fds stored
  DzHashmap
      fd_data;  // Keys are fds, value is FdData. Stores the fd type,
                // and any data needed to be associated with it
  const FileModule *file_module;  // For IO Operations
} FdPool;

typedef struct FdTotalInfo {
  const FdData *data;
  const struct pollfd *event_info;
} FdTotalInfo;

// Initialize an FdPool.
// Arguments:
//  socket_fd: Socket file descriptor to initialize with
//  file_module: pointer to a test filemodule. If NULL, uses the
//  system default
// Must call fdpool_free after init
extern void fdpool_init(FdPool *self, const int socket_fd,
                        const FileModule *file_module);

// Must be called after init
extern void fdpool_free(FdPool *self);

// Returns the amount of file descriptors being monitored in the pool
extern size_t fdpool_size(const FdPool *self);

// Returns if the current fd is ready or not
extern bool fdpool_is_fd_ready(const FdPool *self,
                               const size_t index);

// Adds a new fd with data to the pool
extern void fdpool_add_fd(FdPool *self, const size_t fd,
                          const enum FdDataType datatype);

// Adds a new fd with associated data
extern void fdpool_add_fd_with_data(FdPool *self, const size_t fd,
                                    const FdData *data);

// Removes the specified fd
extern void fdpool_remove_fd(FdPool *self, size_t fd);

// Polls each fd and determines if it is ready for an IO Operation
// If so, then fdpool_is_fd_ready will return true
extern int fdpool_select_ready(FdPool *self, unsigned int timeout);

// Returns a readonly pointer to the file data and event data of the
// fd
extern FdTotalInfo fdpool_get_data(FdPool *self, size_t fd);

extern void fdpool_set_file_data(FdPool *self, size_t fd,
                                 const FdDataFile fd_data_file);
