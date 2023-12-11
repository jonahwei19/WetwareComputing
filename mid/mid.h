#include <stdbool.h>
#define ADDR_MAX_LEN 64
#define HEARTBEAT_BUFFER_SIZE 64
#define MSG_ENVELOPE_BUFFER_SIZE 8
#define MSG_HEADER_BUFFER_SIZE 512
#define MSG_DATA_BUFFER_SIZE 2048

typedef struct {
  int port;
  bool isverbose;
} flags_t;

typedef struct {
  flags_t *p_flags;
  void *context;
  char *addr;
} socket_thread_args;

void parse_args(flags_t *, int, char *const[]);

void *requester_loop(void *);

void *subscriber_loop(void *);
