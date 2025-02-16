#include "file_module.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

const FileModule SYS_FILE_MODULE = {.open = open,
                                    .close = close,
                                    .read = read,
                                    .write = write,
                                    .select = select,
                                    .accept = accept,
                                    .recv = recv,
                                    .send = send,
                                    .poll = poll};
