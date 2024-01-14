#include "signal_handler.hpp"

#include <csignal>
#include <cstdio>

inline void set_RIP() { RIP = 1; }

sig_atomic_t RIP = 0;

static void signal_handler(int signal_num) { set_RIP(); }

void signal_handler_setup() {
  struct sigaction action;
  action.sa_handler = signal_handler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
}
