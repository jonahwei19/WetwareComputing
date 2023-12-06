#include <zmq.h>

#include "util.h"
#include "JSON/cJSON.h"

int compare_float(const void *num1, const void *num2) {
  if (*(float *)num1 > *(float *)num2)
    return 1;
  else
    return -1;
}

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

void parse_msg_header(char *msg_header_buf, header_metadata *metadata) {
  cJSON *j_json = cJSON_Parse(msg_header_buf);
  cJSON *j_content = cJSON_GetObjectItemCaseSensitive(j_json, "content");

  cJSON *j_data_size = cJSON_GetObjectItemCaseSensitive(j_json, "data_size");
  cJSON *j_timestamp = cJSON_GetObjectItemCaseSensitive(j_json, "timestamp");
  metadata->data_size = j_data_size->valueint;
  metadata->timestamp = j_timestamp->valuedouble;

  cJSON *j_channel_num =
      cJSON_GetObjectItemCaseSensitive(j_content, "channel_num");
  cJSON *j_num_samples =
      cJSON_GetObjectItemCaseSensitive(j_content, "num_samples");
  metadata->channel_num = j_channel_num->valueint;
  metadata->num_samples = j_num_samples->valueint;

  cJSON_Delete(j_json);
}
