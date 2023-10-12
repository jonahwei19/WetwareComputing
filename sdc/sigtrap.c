/* (Code copied from ZeroMQ - The Guide)
 * given pointer to array 2 of int, syscall pipe(array) creates a pipe and sets
 * members of array to fd for the read and write end of the pipe: array[0] read
 * end of the pipe; array[1] write end of the pipe.
 *
 * s_setup sets up the pipe and designates s_signal_handler as the handler for
 * SIGINT and SIGTERM. s_signal_handler writes to the pipe when it intercepts
 * either signal.
 *
 * main retrieves fd for the read end of the self-pipe via get_selfpipe_fd.
 * Then, zmq_poll polls a zmq_pollitem defined for it (.fd = selfpipe_read_fd,
 * .event = ZMQ_POLLIN). When s_signal_handler writes S_NOTIFY_MSG to the pipe,
 * expression (zmq_pollitem.revents & ZMQ_POLLIN) evaluates to a truthy value.
 *
 * More info:
 * https://zguide.zeromq.org/docs/chapter2/#Handling-Interrupt-Signals
 * http://api.zeromq.org/master:zmq-poll
 */
#include "sigtrap.h"
#define S_NOTIFY_MSG " "
#define S_ERROR_MSG "Error while writing to self-pipe.\n"

static int g_selfpipe_fds[2];
static bool g_is_setup = false;

static void s_signal_handler(int signal_value) {
  int rc = write(g_selfpipe_fds[1], S_NOTIFY_MSG, sizeof(S_NOTIFY_MSG));
  if (rc != sizeof(S_NOTIFY_MSG)) {
    write(STDOUT_FILENO, S_ERROR_MSG, sizeof(S_ERROR_MSG) - 1);
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
  printf("%d %d\n", index, g_selfpipe_fds[index]);
  return g_selfpipe_fds[index];
}
