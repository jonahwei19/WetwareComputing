#include "util.h"
#include <zmq.h>

int recv_string(void *socket, char *buffer, size_t buffer_size, int flags) {
  int rc;
  rc = zmq_recv(socket, buffer, buffer_size, flags);
  if (rc == -1)
    return rc;
  if (rc < buffer_size)
    buffer[rc] = '\0';
  else
    buffer[buffer_size] = '\0';
  return rc;
}
