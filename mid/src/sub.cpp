#include "sub.hpp"
#include "electrode.hpp"
#include "signal_handler.hpp"
#include "time_utils.hpp"

#include <array>
#include <condition_variable>
#include <deque>
#include <format>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

std::mutex mtx;
std::condition_variable cv;

std::optional<Spike> ParseToSpike(const std::string str_msg_header,
                                  const bool is_verbose) {
  using nlohmann::json;
  json msg_header = json::parse(str_msg_header);
  if (msg_header["type"] != "spike") {
    if (is_verbose) {
      LogText("NOT A SPIKE");
    }
    return {};
  }
  return Spike{msg_header["timestamp"], msg_header["spike"]["electrode"]};
}

/* try consuming the frontal spike future, otherwise wait for its result to
 * become available */
void ConsumeLoop(SpikeFutureDeque &spike_future_deque,
                 const TriggerConditions trigger_conditions,
                 const bool is_verbose) {
  std::map<std::string, Electrode> electrode_map;
  while (!RIP) {
    std::unique_lock lk(mtx);
    cv.wait(lk);
    lk.unlock();
    while (!spike_future_deque.empty()) {
      auto optional_spike = spike_future_deque.front().get();
      if (optional_spike.has_value()) {
        auto new_spike = optional_spike.value();
        if (!electrode_map.contains(new_spike.electrode_name)) {
          electrode_map.emplace(new_spike.electrode_name,
                                Electrode(new_spike, trigger_conditions));
        }
        electrode_map.at(new_spike.electrode_name)
            .UpdateFrequencyScore(new_spike);
      }
      {
        std::lock_guard pop_lk(mtx);
        spike_future_deque.pop_front();
      }
    }
  }
}

/* receive zmq messages and append spike futures to spike_future_deque one at a
 * time */
void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose,
                   const TriggerConditions trigger_conditions) {
  /* message has 3 parts: envelope ("EVENT" or "DATA"), header (json containing
   * metadata), body (binary)
   */
  constexpr auto kFilterString{"EVENT"};
  std::array<zmq::message_t, kNumMessages> msg_parts;
  SpikeFutureDeque spike_future_deque;
  zmq::socket_t sock(ctx, zmq::socket_type::sub);
  sock.set(zmq::sockopt::subscribe, kFilterString);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);
  std::thread consumer_thread(ConsumeLoop, std::ref(spike_future_deque),
                              trigger_conditions, is_verbose);
  try {
    while (!RIP) {
      auto it = msg_parts.begin();
      auto recv_result = zmq::recv_multipart_n(sock, it, kNumMessages);
      if (recv_result) {
        auto spike_future =
            std::async(ParseToSpike, msg_parts[1].to_string(), is_verbose);
        std::unique_lock lk(mtx);
        bool wake_needed = spike_future_deque.empty();
        spike_future_deque.push_back(spike_future.share());
        lk.unlock();
        if (wake_needed) {
          cv.notify_one();
        }
      } else if (is_verbose) {
        LogText("TIMEOUT");
      }
    }
    if (spike_future_deque.empty()) {
      cv.notify_one();
    }
    if (consumer_thread.joinable()) {
      consumer_thread.join();
    }
  } catch (std::exception &e) {
    LogText(std::format("Exception while receiving data: {}", e.what()));
    RIP = 1;
  }
}
