#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void s_signal_handler(int);

void s_setup();

int get_selfpipe_fd(int);
