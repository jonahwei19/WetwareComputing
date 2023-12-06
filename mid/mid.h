#include <stdbool.h>
#define ADDR_MAX_LEN 32
#define SMALL_BUFFER_SIZE 512
#define LARGE_BUFFER_SIZE 4096
#define HEARTBEAT_BUFFER_SIZE 32
#define MSG_ENVELOPE_BUFFER_SIZE 8
#define MSG_HEADER_BUFFER_SIZE 512
#define MSG_DATA_BUFFER_SIZE 8192

typedef struct {
  int factor;
  int port;
  bool isverbose;
} mid_flags;

typedef struct {
  mid_flags *flags;
  void *context;
  char *addr;
} socket_thread_args;

void parse_args(mid_flags *, int, char *const[]);

void *requester_loop(void *);

void *subscriber_loop(void *);
