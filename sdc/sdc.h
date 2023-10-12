#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <zmq.h>
#define ADDR_MAX_LEN 30
#define BUFFER_SIZE 500

typedef struct {
  int port;
  bool isverbose;
} sdc_flags;

typedef struct {
  sdc_flags *flags;
  void *context;
  char *addr;
} socket_thread_data;

void parse_args(sdc_flags *, int, char *const[]);

void requester_loop(socket_thread_data *);
