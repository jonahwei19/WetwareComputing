/* (Code copied from ZeroMQ - The Guide)
 * given pointer to array 2 of int, syscall pipe(array) creates a pipe and sets
 * members of array to fd for the read and write end of the pipe: array[0] read
 * end of the pipe; array[1] write end of the pipe.
 *
 * s_setup sets up the pipe and designates s_signal_handler as the handler for
 * SIGINT and SIGTERM. s_signal_handler writes to the pipe when it intercepts
 * either signal.
 *
 * main could retrieve fd for the read end of the self-pipe via get_selfpipe_fd.
 *
 * More info:
 * https://zguide.zeromq.org/docs/chapter2/#Handling-Interrupt-Signals
 */
#include "sigtrap.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_selfpipe_fds[2];
static bool g_is_setup = false;

static void s_signal_handler(int signal_value) {
  int rc = write(g_selfpipe_fds[1], S_NOTIFY_MSG, S_NOTIFY_MSG_SIZE);
  if (rc != S_NOTIFY_MSG_SIZE) {
    write(STDOUT_FILENO, S_ERROR_MSG, S_ERROR_MSG_SIZE);
    exit(EXIT_FAILURE);
  }
}

void s_setup() {
  if (g_is_setup) {
    exit(EXIT_FAILURE);
  }
  // create self-pipe and populate g_selfpipe_fds
  int rc = pipe(g_selfpipe_fds);
  if (rc != 0) {
    perror("Creating self-pipe");
    exit(EXIT_FAILURE);
  }

  // set to nonblock
  for (int i = 0; i < 2; i++) {
    int flags = fcntl(g_selfpipe_fds[i], F_GETFL, 0);
    if (flags < 0) {
      perror("fcntl(F_GETFL)");
      exit(EXIT_FAILURE);
    }
    rc = fcntl(g_selfpipe_fds[i], F_SETFL, flags | O_NONBLOCK);
    if (rc != 0) {
      perror("fcntl(F_SETFL)");
      exit(EXIT_FAILURE);
    }
  }

  // designate custom handler for SIGINT and SIGTERM
  struct sigaction action;
  action.sa_handler = s_signal_handler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  g_is_setup = true;
}

int get_selfpipe_fd(int index) {
  if (index < 0 || index > 1 || !g_is_setup)
    exit(EXIT_FAILURE);
  return g_selfpipe_fds[index];
}
