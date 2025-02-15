#include "interrupt.h"

#include "core.h"

volatile sig_atomic_t is_running = 1;
static volatile sig_atomic_t interrupt_in_progress = 0;

void interrupt_signal_handler(const int sig) {
  if (interrupt_in_progress) {
    raise(sig);
  }
  DZ_INFO("SIGINT Interrupt Signal Caught");
  interrupt_in_progress = 1;
  // Cleanup
  is_running = 0;
  /*signal(sig, SIG_DFL);*/
  /*raise(sig);*/
}
