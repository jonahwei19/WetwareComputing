#ifndef MID_SUB_H_
#define MID_SUB_H_

#include "electrode.hpp"

#include <array>
#include <deque>
#include <future>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>
#include <zmq.hpp>

inline constexpr int kNumMessages = 3;

typedef std::shared_future<std::optional<Spike>> SpikeFuture;

typedef std::deque<SpikeFuture> SpikeFutureDeque;

std::optional<Spike> ParseToSpike(const std::string str_msg_header,
                                  const bool is_verbose);

void ConsumeLoop(SpikeFutureDeque &spike_future_deque,
                 const TriggerConditions trigger_conditions,
                 const bool is_verbose);

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose,
                   const TriggerConditions trigger_conditions);
#endif
