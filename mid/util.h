#include <stdlib.h>

typedef struct {
  // level 1
  size_t data_size;
  double timestamp;
  // level 2 under "content"
  int channel_num;
  int num_samples;
} header_metadata;

int compare_float(const void *, const void *);

int recv_string(void *, char *, size_t, int);

void parse_msg_header(char *, header_metadata *);
