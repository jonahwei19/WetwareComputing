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

#include "mid.h"
#include "sigtrap.h"
#include "util.h"
#include "JSON/cJSON.h"

volatile bool RIP;

void parse_args(mid_flags *flags, int argc, char *const argv[]) {
  int option;
  while ((option = getopt(argc, argv, "f:p:v")) != -1) {
    switch (option) {
    case 'f':
      flags->factor = strtol(optarg, NULL, 0);
      break;
    case 'p':
      flags->port = strtol(optarg, NULL, 0);
      break;
    case 'v':
      flags->isverbose = true;
      break;
    case '?':
      printf("flags: -f threshold rescaling factor -p port number -v verbose");
      exit(EXIT_FAILURE);
    }
  }
}

void *requester_loop(void *p_args) {
  socket_thread_args *thread_args = (socket_thread_args *)p_args;
  int rc;
  int linger_millisecond = 2000;
  bool isverbose = thread_args->flags->isverbose;
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

void *subscriber_loop(void *p_args) {
  socket_thread_args *thread_args = (socket_thread_args *)p_args;
  int rc;
  int factor = thread_args->flags->factor;
  bool isverbose = thread_args->flags->isverbose;
  char filter[] = "DATA";
  char msg_envelope_buf[MSG_ENVELOPE_BUFFER_SIZE];
  char msg_header_buf[MSG_HEADER_BUFFER_SIZE];
  header_metadata metadata;

  float *msg_data_buf, *absd;
  msg_data_buf = malloc(MSG_DATA_BUFFER_SIZE);
  absd = malloc(MSG_DATA_BUFFER_SIZE);

  void *subscriber = zmq_socket(thread_args->context, ZMQ_SUB);
  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, &filter, sizeof(filter));
  zmq_connect(subscriber, thread_args->addr);

  int n = 0;
  while (true) {
    if (RIP) {
      free(msg_data_buf);
      free(absd);
      zmq_close(subscriber);
      printf("Subscriber thread closed.\n");
      return NULL;
    }
    switch (n) {
    case 0:
      rc = recv_string(subscriber, msg_envelope_buf, MSG_ENVELOPE_BUFFER_SIZE,
                       ZMQ_DONTWAIT);
      break;
    case 1:
      rc = recv_string(subscriber, msg_header_buf, MSG_HEADER_BUFFER_SIZE,
                       ZMQ_DONTWAIT);
      if (rc != -1) {
        parse_msg_header(msg_header_buf, &metadata);
        if (isverbose)
          printf("[msg_header_buf]\n%s\n", msg_header_buf);
        if (metadata.data_size > metadata.num_samples * sizeof(msg_data_buf)) {
          msg_data_buf = realloc(msg_data_buf, metadata.data_size);
          absd = realloc(absd, metadata.data_size);
        }
      }
      break;
    case 2:
      rc = zmq_recv(subscriber, msg_data_buf, metadata.data_size, ZMQ_DONTWAIT);
      if (rc == -1)
        break;
      double mad = 0.0;
      long double total = 0.0;
      int channel_num = metadata.channel_num;
      int num_samples = metadata.num_samples;
      for (int i = 0; i < num_samples; i++) {
        total += msg_data_buf[i];
        if (isverbose) {
          printf("%d:%f,total=%Lf\n", i, msg_data_buf[i], total);
        }
      }
      float mean = total / num_samples;
      for (int i = 0; i < num_samples; i++) {
        absd[i] = fabs(msg_data_buf[i] - mean);
      }
      qsort(absd, num_samples, sizeof(float), compare_float);
      if (num_samples % 2 == 1)
        mad = absd[num_samples / 2 + 1];
      else
        mad = absd[num_samples / 2];
      printf("%lld[%d]:\nmean = %f, mad = %f, threshold = %f\n\n",
             (long long)metadata.timestamp, channel_num, mean, mad,
             mean - factor * mad);
      for (int i = 0; i < num_samples; i++)
        absd[i] = 0.0;
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
  mid_flags flags;
  flags.factor = 6;
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
  socket_thread_args req_thread_data = {
      .flags = &flags, .context = context, .addr = req_addr};
  socket_thread_args sub_thread_data = {
      .flags = &flags, .context = context, .addr = sub_addr};
  pthread_create(&req_thr, NULL, requester_loop, &req_thread_data);
  pthread_create(&sub_thr, NULL, subscriber_loop, &sub_thread_data);

  s_setup();
  int selfpipe_read_fd = get_selfpipe_fd(0);

  // 1 is the size of SIGINT, SIGTERM in int
  char trash_buf[1];
  struct pollfd fds[1] = {{.fd = selfpipe_read_fd, .events = POLLIN}};
  while (true) {
    // zmq_poll is also an option, but unnecessary here.
    int rc = poll(fds, 1, -1);
    read(selfpipe_read_fd, trash_buf, 1);
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
