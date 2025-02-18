#include "file_module.h"
#include <cstdlib>
#include <errno.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

extern "C" {
#include "start.c"
}

volatile sig_atomic_t is_running = 1;

static const short PORT = 8083;
#define N_CLIENTS 100
#define N_REQS_PER_CLIENT 1000

// Sleep for 1 second, then kill the test
void *server_handle(void *data) {
  start(PORT, RESOURCES_PATH, &is_running, 100, &SYS_FILE_MODULE);
  return NULL;
}

static const char *REQUEST_TO_SEND =
    "GET /test.html undefined"
    "Host: localhost:8080"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:135.0) "
    "Gecko/20100101 Firefox/135.0"
    "Accept: "
    "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
    "Accept-Language: en-US,en;q=0.5"
    "Accept-Encoding: gzip, deflate, br, zstd"
    "Connection: keep-alive"
    "Upgrade-Insecure-Requests: 1"
    "Sec-Fetch-Dest: document"
    "Sec-Fetch-Mode: navigate"
    "Sec-Fetch-Site: none"
    "Sec-Fetch-User: ?1"
    "Priority: u=0, i";

static const char *EXPECTED_RESPONSE_HEADER = "HTTP/1.1 200 OKAY\n"
                                              "Content-length: 137\n";
static const size_t EXPECTED_RES_HEADER_LEN = strlen(EXPECTED_RESPONSE_HEADER);

bool client_spam(void *_) {
  unsigned short port = PORT;
  struct hostent *hostname;
  struct sockaddr_in server;
  int socket_fd;

  hostname = gethostbyname("localhost");
  if (!hostname) {
    fprintf(stderr, "hostname failed.\n");
    return false;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);
  char buffer[500] = {0};
  for (size_t i = 0; i < N_REQS_PER_CLIENT; i++) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      fprintf(stderr, "Bad socket. errno %d\n", errno);
      return false;
    }
    if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Bad connect. errno %d\n", errno);
      return false;
    }
    int sent_err =
        send(socket_fd, REQUEST_TO_SEND, strlen(REQUEST_TO_SEND), 0) < 0;
    if (sent_err == -1) {
      fprintf(stderr, "Bad send. errno %d\n", errno);
      return false;
    }
    int result = recv(socket_fd, buffer, sizeof(buffer), 0);
    if (result < 0) {
      fprintf(stderr, "RECV bad. Result %d, Errno %d\n", result, errno);
      sleep(1);
      return false;
    }
    if (0 !=
        strncmp(buffer, EXPECTED_RESPONSE_HEADER, EXPECTED_RES_HEADER_LEN)) {
      fprintf(stderr, "Not equal.\nExpected Result %s\nActual result %s\n\n\n",
              EXPECTED_RESPONSE_HEADER, buffer);
      return false;
    }
    close(socket_fd);
  }
  return true;
}

TEST(Server, Spam) {
  pthread_t server_thread;
  pthread_t client_threads[N_CLIENTS];
  pthread_create(&server_thread, 0, server_handle, (void *)NULL);
  for (size_t i = 0; i < N_CLIENTS; i++) {
    pthread_create(&client_threads[i], 0, (void *(*)(void *))client_spam,
                   (void *)NULL);
  }
  for (size_t i = 0; i < N_CLIENTS; i++) {
    bool result = false;
    pthread_join(client_threads[i], (void **)&result);
    ASSERT_TRUE(result);
  }
  is_running = 0;
  /*pthread_join(server_thread, NULL);*/
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
