#include "sub.hpp"
#include "electrode.hpp"
#include "signal_handler.hpp"
#include "time_utils.hpp"

#include <array>
#include <format>
#include <iostream>
#include <map>
#include <string>

#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose,
                   const TriggerConditions trigger_conditions) {
  /* message has 3 parts: envelope ("EVENT" or "DATA"), header (json containing
   * metadata), body (binary)
   */
  using nlohmann::json;
  constexpr auto kNumMessages = 3;
  constexpr auto kFilterString{"EVENT"};
  int frequency_score = 0;
  json msg_header;
  std::array<zmq::message_t, kNumMessages> msg_parts;
  std::map<std::string, Electrode> electrode_map;
  zmq::socket_t sock(ctx, zmq::socket_type::sub);
  sock.set(zmq::sockopt::subscribe, kFilterString);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);
  try {
    while (!RIP) {
      auto it = msg_parts.begin();
      auto recv_result = zmq::recv_multipart_n(sock, it, kNumMessages);
      if (!recv_result) {
        if (is_verbose) {
          LogText("TIMEOUT");
        }
        continue;
      }
      msg_header = json::parse(msg_parts[1].to_string());
      if (msg_header["type"] != "spike") {
        if (is_verbose) {
          LogText("NOT A SPIKE");
        }
        continue;
      }
      Spike new_spike = {msg_header["timestamp"],
                         msg_header["spike"]["electrode"]};
      if (!electrode_map.contains(new_spike.electrode_name)) {
        electrode_map.emplace(new_spike.electrode_name,
                              Electrode(new_spike, trigger_conditions));
      }
      electrode_map.at(new_spike.electrode_name)
          .UpdateFrequencyScore(new_spike);
    }
  } catch (std::exception &e) {
    LogText(std::format("Exception while receiving data: {}", e.what()));
    RIP = 1;
    return;
  }
}
