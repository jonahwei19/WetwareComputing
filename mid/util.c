#include "util.h"
#include "JSON/cJSON.h"
#include <string.h>
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

int parse_msg_header_buf(char *msg_header_buf, msg_header_t *p_msg_header) {
  cJSON *j_json = cJSON_Parse(msg_header_buf);

  cJSON *j_msg_type = cJSON_GetObjectItemCaseSensitive(j_json, "type");
  cJSON *j_timestamp = cJSON_GetObjectItemCaseSensitive(j_json, "timestamp");

  strncpy(p_msg_header->msg_type, j_msg_type->valuestring, MSG_TYPE_SIZE);
  p_msg_header->timestamp = j_timestamp->valuedouble;

  if (strcmp(j_msg_type->valuestring, "spike") != 0) {
    cJSON_Delete(j_json);
    return -1;
  }

  cJSON *j_spike = cJSON_GetObjectItemCaseSensitive(j_json, "spike");
  cJSON *j_electrode_name =
      cJSON_GetObjectItemCaseSensitive(j_spike, "electrode");
  strncpy(p_msg_header->electrode_name, j_electrode_name->valuestring,
          ELECTRODE_NAME_SIZE);

  cJSON_Delete(j_json);
  return 0;
}
