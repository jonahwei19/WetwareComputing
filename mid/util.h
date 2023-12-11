#include <stdlib.h>
#define ERROR -1
#define MSG_TYPE_SIZE 32
#define ELECTRODE_NAME_SIZE 32

typedef struct {
  // level 1
  char msg_type[MSG_TYPE_SIZE];
  double timestamp;
  // level 2 under "spike"
  char electrode_name[ELECTRODE_NAME_SIZE];
} msg_header_t;

int recv_string(void *, char *, size_t, int);

int parse_msg_header_buf(char *, msg_header_t *);
