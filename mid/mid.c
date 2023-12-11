#include "mid.h"
#include "sigtrap.h"
#include "util.h"
#include "JSON/cJSON.h"
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <zmq.h>

volatile bool RIP;

void parse_args(flags_t *p_flags, int argc, char *const argv[]) {
  int option;
  while ((option = getopt(argc, argv, "f:p:v")) != -1) {
    switch (option) {
    case 'p':
      p_flags->port = strtol(optarg, NULL, 0);
      break;
    case 'v':
      p_flags->isverbose = true;
      break;
    case '?':
      printf("flags: -p port number -v verbose");
      exit(EXIT_FAILURE);
    }
  }
}

void *requester_loop(void *p_args) {
  socket_thread_args *thread_args = (socket_thread_args *)p_args;
  int rc;
  int linger_millisecond = 2000;
  bool isverbose = thread_args->p_flags->isverbose;
  char buf[HEARTBEAT_BUFFER_SIZE];

  // Generate uuid
  uuid_t binuuid;
  char struuid[UUID_STR_LEN];
  uuid_generate_random(binuuid);
  uuid_unparse_upper(binuuid, struuid);

  // Generate JSON string
  cJSON *application = cJSON_CreateString("mid");
  cJSON *uuid = cJSON_CreateString(struuid);
  cJSON *type = cJSON_CreateString("heartbeat");
  cJSON *heartbeat = cJSON_CreateObject();
  cJSON_AddItemToObject(heartbeat, "application", application);
  cJSON_AddItemToObject(heartbeat, "uuid", uuid);
  cJSON_AddItemToObject(heartbeat, "type", type);
  char *str_heartbeat = cJSON_Print(heartbeat);
  cJSON_Delete(heartbeat);

  void *requester = zmq_socket(thread_args->context, ZMQ_REQ);
  zmq_setsockopt(requester, ZMQ_LINGER, &linger_millisecond,
                 sizeof(linger_millisecond));
  zmq_connect(requester, thread_args->addr);

  while (true) {
    if (RIP) {
      zmq_close(requester);
      printf("Heart stopped beating.\n");
      return NULL;
    }
    zmq_send(requester, strdup(str_heartbeat), strlen(str_heartbeat), 0);
    sleep(2);
    rc = recv_string(requester, buf, HEARTBEAT_BUFFER_SIZE, ZMQ_DONTWAIT);
    if (isverbose && rc != -1)
      printf("%s\n", buf);
  }
}

void *subscriber_loop(void *voidp_args) {
  socket_thread_args *p_args = (socket_thread_args *)voidp_args;
  int rc, parse_rc;
  bool isverbose = p_args->p_flags->isverbose;
  char filter[] = "EVENT";
  char msg_envelope_buf[MSG_ENVELOPE_BUFFER_SIZE];
  char msg_header_buf[MSG_HEADER_BUFFER_SIZE];
  msg_header_t msg_header;

  float *msg_data_buf;
  msg_data_buf = malloc(MSG_DATA_BUFFER_SIZE);

  void *subscriber = zmq_socket(p_args->context, ZMQ_SUB);
  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, sizeof(filter));
  zmq_connect(subscriber, p_args->addr);

  int n = 0;
  while (true) {
    if (RIP) {
      free(msg_data_buf);
      zmq_close(subscriber);
      printf("Subscriber thread closed.\n");
      return NULL;
    }
    switch (n) {
    case 0:
      rc = recv_string(subscriber, msg_envelope_buf, MSG_ENVELOPE_BUFFER_SIZE,
                       ZMQ_DONTWAIT);
      if (rc != -1 && isverbose) {
        printf("[msg_envelope]%s\n", msg_envelope_buf);
      }
      break;
    case 1:
      rc = recv_string(subscriber, msg_header_buf, MSG_HEADER_BUFFER_SIZE,
                       ZMQ_DONTWAIT);
      if (rc != -1) {
        parse_rc = parse_msg_header_buf(msg_header_buf, &msg_header);
        if (isverbose)
          printf("[msg_header]\n%s\n", msg_header_buf);
      }
      break;
    case 2:
      rc = zmq_recv(subscriber, msg_data_buf, MSG_DATA_BUFFER_SIZE,
                    ZMQ_DONTWAIT);
      if (rc == -1 || parse_rc == -1)
        break;
      for (int i = 0; i < 85; i++) {
        printf("%d: %f\n", i, msg_data_buf[i]);
      }
      memset(msg_data_buf, 0, MSG_DATA_BUFFER_SIZE);
    }
    if (rc == -1 || n == 2)
      n = 0;
    else
      n++;
  }
}

int main(int argc, char **argv) {
  RIP = false;
  // set flags
  flags_t flags;
  flags.port = 5556;
  flags.isverbose = false;
  parse_args(&flags, argc, argv);

  // set addr
  char req_addr[ADDR_MAX_LEN], sub_addr[ADDR_MAX_LEN];
  sprintf(req_addr, "tcp://localhost:%d", flags.port + 1);
  sprintf(sub_addr, "tcp://localhost:%d", flags.port);
  printf("req-rep (heartbeating) addr: %s\n", req_addr);
  printf("pub-sub addr: %s\n", sub_addr);

  void *context = zmq_ctx_new();

  // create threads
  pthread_t req_thr, sub_thr;
  socket_thread_args req_thread_args = {
      .p_flags = &flags, .context = context, .addr = req_addr};
  socket_thread_args sub_thread_args = {
      .p_flags = &flags, .context = context, .addr = sub_addr};
  pthread_create(&req_thr, NULL, requester_loop, &req_thread_args);
  pthread_create(&sub_thr, NULL, subscriber_loop, &sub_thread_args);

  s_setup();
  int selfpipe_read_fd = get_selfpipe_fd(0);

  char buf[S_NOTIFY_MSG_SIZE];
  struct pollfd fds[1] = {{.fd = selfpipe_read_fd, .events = POLLIN}};
  while (true) {
    // zmq_poll is also an option, but unnecessary here.
    int rc = poll(fds, 1, -1);
    read(selfpipe_read_fd, buf, S_NOTIFY_MSG_SIZE);
    /* "On success, poll returns a non-negative value [...] zero
     * indicates that the system call timed out [...]" (see `man poll`)
     */
    if (rc > 0)
      break;
  }
  printf("Exiting gracefully...\n");
  RIP = true;
  pthread_join(req_thr, NULL);
  pthread_join(sub_thr, NULL);
  zmq_ctx_term(context);
  return EXIT_SUCCESS;
}
