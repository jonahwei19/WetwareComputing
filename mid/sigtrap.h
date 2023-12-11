#define S_NOTIFY_MSG "a"
#define S_ERROR_MSG "Error while writing to self-pipe.\n"
#define S_NOTIFY_MSG_SIZE 1
#define S_ERROR_MSG_SIZE 34

static void s_signal_handler(int);

void s_setup();

int get_selfpipe_fd(int);
