#ifndef MID_SUB_H_
#define MID_SUB_H_

#include "electrode.hpp"

#include <array>
#include <string>

#include <nlohmann/json.hpp>
#include <zmq.hpp>

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose,
                   const TriggerConditions trigger_conditions);
#endif
