#ifndef MID_REQ_H_
#define MID_REQ_H_
#include <string>
#include <zmq.hpp>
std::string GenerateUuid();
std::string GenerateHeartbeat(const std::string uuid);
void RequestLoop(const std::string addr, zmq::context_t &ctx,
                 const bool is_verbose);
#endif
