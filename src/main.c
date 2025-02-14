#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "arguments.h"
#include "core.h"
#include "pool.h"
#include "server.h"
#include "arena.h"

const char *BAD_REQ_ERR = "HTTP/1.1 400 Bad Request\r\n\r\n";
const char *NOT_FOUND_ERR = "HTTP/1.1 404 Not Found\r\n\r\n";
const char *OKAY_200 =
    "HTTP/1.1 200\r\n"
    "Cache-Control: no-transform\r\n"
    "Content-Type: text/event-stream\r\n"
    "Connection: keep-alive\r\n"
    "retry: 10000\r\n";

static const char *HTTP_SPACE_DELIMITER = " ";
static const char *HTTP_LINE_DELIMITER = "\n";

#define BUFFER_SIZE 8192
static DZArena arena;

volatile sig_atomic_t fatal_error_in_progress = 0;
volatile sig_atomic_t is_running = 1;
void interrupt_signal_handler(const int sig) {
  if (fatal_error_in_progress) {
    raise(sig);
  }
  printf("\nINT SIGNAL CAUGHT\n");
  fatal_error_in_progress = 1;
  // Cleanup
  is_running = 0;
  /*signal(sig, SIG_DFL);*/
  /*raise(sig);*/
}

int main(const int argc, const char **argv) {
#ifdef _DEBUG
  printf("Debug signal handling mode enabled\n");
  signal(SIGINT, interrupt_signal_handler);
#endif
  const Arguments args = args_parse(argc, argv);
  if (args.error) {
    args_print_usage();
    exit(EXIT_FAILURE);
  }
  DZArena arena = dz_arena_init(0);
  if (arena.error) {
    exit_error("Arena could not malloc. Error: %d. Error number: %d\n", arena.error, errno);
  }
  static char* buffer = NULL;
  Server server = server_create(args.port);
  printf("Accepting connections on port %d\n", server.port);

  FdPool fdpool;
  fdpool_init(&fdpool, server.fd);

  while (is_running) {
    buffer = (char*)dz_arena_alloc(&arena, BUFFER_SIZE + 1); 
    int nready = fdpool_select_ready(&fdpool);
    if (nready < 0) {
      fprintf(stderr, "ERROR With select. Error Number: %d\n", errno);
      continue;
    }

    for (size_t i = 0; i <= fdpool.maxfd && nready > 0; i++) {
      if (!fdpool_is_fd_ready(&fdpool, i)) {
        continue;
      }

      nready--;
      const enum FdDataType fd_type = fdpool_get_fd_type(&fdpool, i);
      if (fd_type == FD_DATA_SERVER) {
        // Accept a new client connection and add it to the pool
        printf("Accepting new connection\n");
        const int client_fd = accept(server.fd, NULL, NULL);
        if (client_fd == -1) {
          if (errno != EWOULDBLOCK) {
            break;
          }
          exit_error("Cannot listen(). Aborting. Error number: %d\n",
                     errno);
        }
        printf("Accepting %d\n", client_fd);
        fdpool_add_fd(&fdpool, client_fd, FD_DATA_CLIENT);
      } else if (fd_type == FD_DATA_CLIENT) {
        // Recieve data from a file descriptor (a client descriptor)
        recv(i, buffer, BUFFER_SIZE, 0);
        buffer[BUFFER_SIZE] = '\0';  // Safety!
        /*printf("Received message %s\n", buffer);*/
        const char *method = strtok(buffer, HTTP_SPACE_DELIMITER);
        if (!method) {
          fprintf(stderr, "Could not get method\n");
          goto close_client_connection;
        }
        printf("Method %s\n", method);

        if (str_eq(method, "GET", BUFFER_SIZE)) {
          static const size_t RESOURCE_PATH_LEN = strlen(RESOURCES_PATH);
          const char *file_name = strtok(NULL, HTTP_SPACE_DELIMITER);
          const size_t file_name_len = strlen(file_name);
          char *full_file_name = (char*)dz_arena_alloc(&arena, file_name_len + RESOURCE_PATH_LEN + 1);
          strncpy(full_file_name, RESOURCES_PATH, RESOURCE_PATH_LEN - 1);
          strcat(full_file_name, file_name);
          printf("Getting file %s\n", full_file_name);
          const int fd_to_read = open(full_file_name, O_RDONLY);
          if (fd_to_read == -1) {
            write(i, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
            fprintf(stderr, "Could not find file. Error number %d\n",
                    errno);
            goto close_client_connection;
          }
          // Set data for FD to read, so select can wait
          fdpool_add_fd(&fdpool, fd_to_read, FD_DATA_FILE);
          const FdDataFile file_data = {.client_fd_to_send = i};
          fdpool_set_file_data(&fdpool, fd_to_read, file_data);
          continue;
        }
      close_client_connection:
        // Only get here if client conneciton must close
        fdpool_remove_fd(&fdpool, i);
        close(i);
      } else if (fd_type == FD_DATA_FILE) {
        // Send file to a client
        const FdDataFile file_data = fdpool_get_file_data(&fdpool, i);
        const int client_fd = file_data.client_fd_to_send;
        const size_t bytes_read = read(i, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
          fprintf(stderr, "Error with read(). Error number: %d\n",
                  errno);
          write(client_fd, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
          goto close_file_client_conn;
        }
        const int n_written = write(client_fd, buffer, bytes_read);
        if (n_written < 0) {
          fprintf(stderr, "Could not write file. Error number: %d\n",
                  errno);
          write(client_fd, NOT_FOUND_ERR, strlen(NOT_FOUND_ERR));
          goto close_file_client_conn;
        }
      close_file_client_conn:
        fdpool_remove_fd(&fdpool, client_fd);
        fdpool_remove_fd(&fdpool, i);
        close(client_fd);
      }
    }
    dz_arena_clear(&arena);
  }
  printf("Exiting...\n");
  dz_arena_free(&arena);
  return EXIT_SUCCESS;
}
