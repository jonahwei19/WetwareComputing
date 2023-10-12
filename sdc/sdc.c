#include "sdc.h"
#include "sigtrap.h"
#include "util.h"
#include "JSON/cJSON.h"
#include <zmq.h>

volatile bool RIP;

void parse_args(sdc_flags *flags, int argc, char *const argv[]) {
  int option;
  while ((option = getopt(argc, argv, "p:v")) != -1) {
    switch (option) {
    case 'p':
      flags->port = strtol(optarg, NULL, 0);
      break;
    case 'v':
      flags->isverbose = true;
      break;
    case '?':
      printf("flags: -p port number -v verbose\n");
      exit(EXIT_FAILURE);
    }
  }
}

void requester_loop(socket_thread_data *socket_data) {
  int rc;
  int linger_millisecond = 2000;
  bool isverbose = socket_data->flags->isverbose;
  char buf[BUFFER_SIZE];

  // Generate uuid
  uuid_t binuuid;
  char struuid[UUID_STR_LEN];
  uuid_generate_random(binuuid);
  uuid_unparse_upper(binuuid, struuid);

  // Generate JSON string
  cJSON *application = cJSON_CreateString("sdc");
  cJSON *uuid = cJSON_CreateString(struuid);
  cJSON *type = cJSON_CreateString("heartbeat");
  cJSON *heartbeat = cJSON_CreateObject();
  cJSON_AddItemToObject(heartbeat, "application", application);
  cJSON_AddItemToObject(heartbeat, "uuid", uuid);
  cJSON_AddItemToObject(heartbeat, "type", type);
  char *str_heartbeat = cJSON_Print(heartbeat);

  void *requester = zmq_socket(socket_data->context, ZMQ_REQ);
  zmq_setsockopt(requester, ZMQ_LINGER, &linger_millisecond,
                 sizeof(linger_millisecond));
  zmq_connect(requester, socket_data->addr);

  while (true) {
    if (RIP) {
      zmq_close(requester);
      printf("Heart stopped beating.\n");
      return;
    }
    zmq_send(requester, strdup(str_heartbeat), strlen(str_heartbeat), 0);
    sleep(2);
    rc = zmq_recv(requester, buf, BUFFER_SIZE, ZMQ_DONTWAIT);
    if (isverbose && rc != -1)
      printf("%s\n", buf);
  }
}

int main(int argc, char **argv) {
  RIP = false;
  // set flags
  sdc_flags flags;
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
  socket_thread_data req_thread_data = {
      .flags = &flags, .context = context, .addr = req_addr};
  socket_thread_data sub_thread_data;
  pthread_create(&req_thr, NULL, requester_loop, &req_thread_data);

  s_setup();
  int selfpipe_read_fd = get_selfpipe_fd(0);

  // 1 is the size of SIGINT, SIGTERM in int
  char trash_buf[1];
  struct pollfd fds[1] = {{.fd = selfpipe_read_fd, .events = POLLIN}};
  while (true) {
    // zmq_poll is also an option, but unnecessary here.
    int rc = poll(fds, 1, -1);
    printf("%d\n", rc);
    read(selfpipe_read_fd, trash_buf, 1);
    /* "On success, poll returns a non-negative value [...] zero
     * indicates that the system call timed out [...]" (see `man poll`)
     */
    if (rc > 0)
      break;
  }
  printf("Exiting gracefully...");
  RIP = true;
  pthread_join(req_thr, NULL);
  printf("out of pthread!\n");
  zmq_ctx_term(context);
  return EXIT_SUCCESS;
}
