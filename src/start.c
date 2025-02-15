#include "start.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "arena.h"
#include "core.h"
#include "pool.h"
#include "server.h"

static const char *BAD_REQ_ERR = "HTTP/1.1 400 Bad Request\r\n\r\n";
static const char *NOT_FOUND_ERR = "HTTP/1.1 404 Not Found\r\n\r\n";
static const char *HTTP_SPACE_DELIMITER = " ";
static const char *HTTP_LINE_DELIMITER = "\n";

#define BUFFER_SIZE 8192

static char *get_absolute_filename(const char *file_name,
                                   DZArena *arena,
                                   const char *resources_path) {
  const size_t resources_path_len = strlen(resources_path);
  const size_t file_name_len = strlen(file_name);
  if (file_name[0] == '/') {
    file_name++;
  }
  char *full_file_name = (char *)dz_arena_alloc(
      arena, file_name_len + resources_path_len + 1);
  if (arena->error) {
    return NULL;
  }
  strncpy(full_file_name, resources_path, resources_path_len);
  strcat(full_file_name, file_name);
  return full_file_name;
}

static char *construct_http_response(DZArena *arena,
                                     const char *content,
                                     const size_t content_size,
                                     const char *content_type) {
  char *response = (char *)dz_arena_alloc(arena, content_size + 1000);
  sprintf(response,
          "HTTP/1.1 %d %s\n"
          "Content-length: %zu\n"
          "Content-Type: %s\n"
          "\n"
          "%s",
          200, "OKAY", content_size, content_type, content);
  return response;
}

int start(const int port, const char *resources_path,
          volatile sig_atomic_t *is_running,
          const unsigned int timeout) {
  DZArena arena = dz_arena_init(0);
  if (arena.error) {
    DZ_ERRORNO("Arena could not malloc");
    exit(EXIT_FAILURE);
  }
  static char *buffer = NULL;
  Server server = server_create(port);
  DZ_INFO("Accepting connections with fd %d on port %d", server.fd,
          server.port);

  FdPool fdpool;
  fdpool_init(&fdpool, server.fd);

  while (*is_running) {
    int nready = fdpool_select_ready(&fdpool, timeout);
    if (nready < 0 && errno != EINTR) {
      DZ_ERRORNO("Error with select()");
      continue;
    }

    for (size_t i = 0; i <= fdpool.maxfd && nready > 0; i++) {
      dz_arena_clear(&arena);
      buffer = (char *)dz_arena_alloc(&arena, BUFFER_SIZE + 1);
      if (!fdpool_is_fd_ready(&fdpool, i)) {
        continue;
      }

      nready--;
      const enum FdDataType fd_type = fdpool_get_fd_type(&fdpool, i);
      if (fd_type == FdDataType_SERVER) {
        // Accept a new client connection and add it to the pool
        const int client_fd = accept(server.fd, NULL, NULL);
        if (client_fd == -1) {
          if (errno != EWOULDBLOCK) {
            break;
          }
          DZ_ERRORNO("Cannot listen(). Aborting");
          exit(EXIT_FAILURE);
        }
        DZ_INFO("Accepted new client %d", client_fd);
        fdpool_add_fd(&fdpool, client_fd, FdDataType_CLIENT);
      } else if (fd_type == FdDataType_CLIENT) {
        const int client_fd = i;
        // Recieve data from a file descriptor (a client descriptor)
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        buffer[BUFFER_SIZE] = '\0';  // Safety!
        const char *method = strtok(buffer, HTTP_SPACE_DELIMITER);
        if (!method) {
          DZ_WARN("Could not parse method from request %s", buffer);
          goto close_client_connection;
        }
        if (str_eq(method, "GET", BUFFER_SIZE)) {
          const char *file_name = strtok(NULL, HTTP_SPACE_DELIMITER);
          const char *full_file_name = get_absolute_filename(
              file_name, &arena, resources_path);
          if (!full_file_name) {
            DZ_ERRORNO("Could not allocate memory for file");
            goto close_client_connection;
          }
          DZ_INFO("Getting file %s", full_file_name);
          const int fd_to_read = open(full_file_name, O_RDONLY);
          if (fd_to_read == -1) {
            write(client_fd, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
            DZ_WARNNO("Could not find file %s", full_file_name);
            goto close_client_connection;
          }
          // Set data for FD to read, so select can wait
          fdpool_add_fd(&fdpool, fd_to_read, FdDataType_FILE);
          const FdDataFile file_data = {.client_fd_to_send =
                                            client_fd};
          fdpool_set_file_data(&fdpool, fd_to_read, file_data);
          continue;
        }
      close_client_connection:
        // Only get here if client conneciton must close
        fdpool_remove_fd(&fdpool, client_fd);
        close(client_fd);
      } else if (fd_type == FdDataType_FILE) {
        // Send file to a client
        const int file_fd = i;
        const FdDataFile file_data =
            fdpool_get_file_data(&fdpool, file_fd);
        const int client_fd = file_data.client_fd_to_send;
        const size_t bytes_read = read(file_fd, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
          DZ_ERRORNO("Error with read()");
          write(client_fd, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
          goto close_file_client_conn;
        }
        {
          const char *http_response = construct_http_response(
              &arena, buffer, bytes_read, "text/html");
          const int n_written =
              write(client_fd, http_response, strlen(http_response));
          if (n_written < 0) {
            char filepath[PATH_MAX];
            DZ_ERRORNO("Could not write() file to client %d",
                       client_fd);
            write(client_fd, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
            goto close_file_client_conn;
          }
        }
      close_file_client_conn:
        fdpool_remove_fd(&fdpool, client_fd);
        fdpool_remove_fd(&fdpool, file_fd);
        close(client_fd);
        close(file_fd);
      }
    }
    dz_arena_clear(&arena);
  }
  // Cleanup
  DZ_INFO("Exiting gracefully...");
  dz_arena_free(&arena);
  for (size_t i = 0; i < fdpool.maxfd; i++) {
    if (!fdpool_contains_fd(&fdpool, i)) {
      continue;
    }
    // Cleanup based on FD type
    enum FdDataType fdtype = fdpool_get_fd_type(&fdpool, i);
    switch (fdtype) {
      case FdDataType_CLIENT:
      case FdDataType_FILE:
      case FdDataType_SERVER:
        close(i);
        break;
      case FdDataType_COUNT:
        break;
    }
  }
  return EXIT_SUCCESS;
}
