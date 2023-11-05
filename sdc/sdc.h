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
#define ADDR_MAX_LEN 32
#define SMALL_BUFFER_SIZE 512
#define LARGE_BUFFER_SIZE 4096
#define MSG_ENVELOPE_BUFFER_SIZE 8
#define MSG_HEADER_BUFFER_SIZE 512
#define MSG_DATA_BUFFER_SIZE 4096

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
void subscriber_loop(socket_thread_data *);
