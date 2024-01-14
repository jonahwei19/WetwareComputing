#ifndef MID_SIGNAL_H_
#define MID_SIGNAL_H_
#include <csignal>
extern sig_atomic_t RIP;
inline void set_RIP();
static void signal_handler(int signal_num);
void signal_handler_setup();
#endif
