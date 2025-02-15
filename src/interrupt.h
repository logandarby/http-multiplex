#pragma once

#include <signal.h>

extern volatile sig_atomic_t is_running;

void interrupt_signal_handler(const int sig);
