#pragma once

#include <signal.h>

// Starts the server on port PORT, and serves files relative to
// resources_path. Returns a succes/failure code
// is_running is meant for signal handling. If this isn't needed, pass
// in NULL
extern int start(const int port, const char* resources_path,
                 volatile sig_atomic_t* is_running,
                 unsigned int timeout);
