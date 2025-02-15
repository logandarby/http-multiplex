#include "pool.h"

#include <bits/types/struct_timeval.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>

#include "core.h"

void fdpool_init(FdPool *self, const int socket_fd) {
  DZ_ASSERT(socket_fd < POOL_FD_SETSIZE, "Socket FD too big");
  memset(self, 0, sizeof(FdPool));
  self->socket_fd = socket_fd;
  self->maxfd = socket_fd;
  FD_ZERO(&self->read_set);
  FD_ZERO(&self->select_ready_set);
  fdpool_add_fd(self, socket_fd, FdDataType_SERVER);
}

void fdpool_add_fd(FdPool *self, const int fd, enum FdDataType type) {
  DZ_ASSERT(fd < POOL_FD_SETSIZE, "Socket FD too big");
  fcntl(fd, F_SETFD, O_NONBLOCK);
  FD_SET(fd, &self->read_set);
  FdData *fd_data = &self->fd_data[fd];
  memset(fd_data, 0, sizeof(*fd_data));
  fd_data->data_type = type;
  self->maxfd = max(self->maxfd, fd);
}

void fdpool_remove_fd(FdPool *self, const int fd) {
  FD_CLR(fd, &self->read_set);
}

int fdpool_select_ready(FdPool *self, unsigned int timeout) {
  memcpy(&self->select_ready_set, &self->read_set,
         sizeof(self->read_set));
  struct timeval time = {.tv_usec = timeout};
  self->nready = select(self->maxfd + 1, &self->select_ready_set,
                        NULL, NULL, &time);
  return self->nready;
}

bool fdpool_contains_fd(FdPool *self, const int fd) {
  return FD_ISSET(fd, &self->read_set);
}

bool fdpool_is_fd_ready(FdPool *self, const int fd) {
  return FD_ISSET(fd, &self->select_ready_set);
}

enum FdDataType fdpool_get_fd_type(FdPool *self, int fd) {
  return self->fd_data[fd].data_type;
}

void fdpool_set_file_data(FdPool *self, int fd_to_read,
                          FdDataFile file_data) {
  const bool is_file_data_type = self->fd_data[fd_to_read].data_type == FdDataType_FILE;
  DZ_ASSERT(is_file_data_type, "Cannot set file data for file descriptor which is not FdDataType_FILE");
  self->fd_data[fd_to_read].fdDataFile = file_data;
}

FdDataFile fdpool_get_file_data(FdPool *self, int fd_to_read) {
  const bool is_file_data_type = self->fd_data[fd_to_read].data_type == FdDataType_FILE;
  DZ_ASSERT(is_file_data_type, "Cannot get file data from file descriptor which is not FdDataType_FILE");
  return self->fd_data[fd_to_read].fdDataFile;
}
