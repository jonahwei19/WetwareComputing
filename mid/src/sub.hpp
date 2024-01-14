#ifndef MID_SUB_H_
#define MID_SUB_H_
#include <string>
#include <zmq.hpp>
void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose);
#endif
