#include "pool.h"

#include <bits/types/struct_timeval.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>

#include "dz_debug.h"
#include "dz_hashmap.h"

void fdpool_init(FdPool *self, const int socket_fd,
                 const FileModule *file_module) {
  DZ_ASSERT(self, "Caller must supply an FDPool to initialize");
  if (!self) {
    return;
  }
  memset(self, 0, sizeof(FdPool));
  self->socket_fd = socket_fd;
  DzHmError error = 0;
  self->fd_data = hm_init(&error);
  DZ_ASSERT(!error);
  self->file_module = (file_module) ? file_module : &SYS_FILE_MODULE;
  // Get linux hard limits for file descriptors and allocate memory
  struct rlimit rlim = {0};
  const int limit_err = getrlimit(RLIMIT_NOFILE, &rlim);
  DZ_ASSERT(limit_err != -1);
  self->fd_limit = rlim.rlim_max;
  self->poll_fd_array = (struct pollfd *)malloc(rlim.rlim_max * sizeof(struct pollfd));
  for (size_t i = 0; i < rlim.rlim_max; i++) {
    self->poll_fd_array[i].fd = -1;
  }
  DZ_ASSERT(self->poll_fd_array, "Could not allocate FdPool");
  self->max_fd = socket_fd;
  fdpool_add_fd(self, socket_fd, FdDataType_SERVER);
}

void fdpool_free(FdPool *self) {
  DZ_ASSERT(self, "Caller must supply a non-null FdPool");
  DZ_ASSERT(self->fd_data);
  DZ_ASSERT(self->poll_fd_array);
  if (!self) {
    return;
  }
  hm_free(self->fd_data);
  free(self->poll_fd_array);
}

int fdpool_select_ready(FdPool *self, unsigned int timeout) {
  DZ_ASSERT(self);
  DZ_TRACE("Selecting");
  return self->file_module->poll(self->poll_fd_array, self->max_fd + 1,
              timeout ? timeout : -1);
}

void fdpool_add_fd_with_data(FdPool *self, const size_t fd, const FdData *data) {
  DZ_ASSERT(self, "Caller must supply an FdPool");
  if (fd >= self->fd_limit) {
    DZ_ERROR("FD supplied is larger than the limit");
    return;
  }
  if (!self) {
    return;
  }
  const struct pollfd event_info = {
      .fd = fd,
      .events = POLLIN,  // Current only supports read events -- no
                         // write or exceptional events
      .revents = 0};
  // Must keep these two arrays in sync always
  self->poll_fd_array[fd] = event_info;
  DzHmError hm_error = 0;
  hm_add(self->fd_data, &fd, sizeof(fd), data, sizeof(*data), &hm_error);
  DZ_ASSERT(!hm_error);
  self->max_fd = max(self->max_fd, fd);
}

void fdpool_add_fd(FdPool *self, const size_t fd,
                   const enum FdDataType datatype) {
  FdData data;
  memset(&data, 0, sizeof(data));
  data.data_type = datatype;
  fdpool_add_fd_with_data(self, fd, &data);
}

size_t fdpool_size(const FdPool *self) {
  DZ_ASSERT(self);
  return hm_count(self->fd_data);
}

 bool fdpool_is_fd_ready(const FdPool *self,
                               const size_t fd) {
  DZ_ASSERT(fd < self->fd_limit, "Index is too big: Index should be less than the max FD limit set by the system");
  if (fd >= self->fd_limit) {
    return false;
  }
  return self->poll_fd_array[fd].revents > 0;
}

 void fdpool_set_file_data(FdPool *self, size_t fd,
                                 const FdDataFile fd_data_file) {
  DZ_ASSERT(self, "Caller must supply an FDPool");
  DZ_ASSERT(fd < self->fd_limit, "Index is too big: Index should be less than the max FD limit set by the system");
  if (!self || fd >= self->fd_limit) {
    return;
  }
  DzHmError hm_error = 0;
  hm_add(self->fd_data, &fd, sizeof(fd), &fd_data_file, sizeof(fd_data_file), &hm_error);
  DZ_ASSERT(!hm_error);
}

FdTotalInfo fdpool_get_data(FdPool *self, size_t fd) {
  DZ_ASSERT(self, "Caller must supply FdPool");
  DZ_ASSERT(fd < self->fd_limit, "Index too big");
  if (!self || fd >= self->fd_limit || self->poll_fd_array[fd].fd == -1) {
    static const FdTotalInfo bad_return_val = {
        .data = NULL,
        .event_info = NULL,
    };
    return bad_return_val;
  }
  const FdTotalInfo return_val = {
      .event_info = &self->poll_fd_array[fd],
      .data = hm_get(self->fd_data, &fd, sizeof(fd))};
  return return_val;
}

void fdpool_remove_fd(FdPool *self, const size_t fd) {
  // TODO: Update maxfd
  DZ_ASSERT(self, "Caller must supply an FdPool");
  DZ_ASSERT(fd < self->fd_limit, "Index is too big: Index should be less than the max FD limit set by the system");
  if (!self || fd >= self->fd_limit) {
    return;
  }
  hm_delete(self->fd_data, &fd, sizeof(fd));
  self->poll_fd_array[fd].fd = -1;
}

