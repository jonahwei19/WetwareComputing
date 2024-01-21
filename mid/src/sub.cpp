#include "sub.hpp"
#include "signal_handler.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose) {
  /* message has 3 parts: envelope ("EVENT" or "DATA"), header (json containing
   * metadata), body (binary)
   */
  using nlohmann::json;
  constexpr auto kNumMessages = 3;
  constexpr auto kFilterString{"EVENT"};
  zmq::socket_t sock(ctx, zmq::socket_type::sub);
  sock.set(zmq::sockopt::subscribe, kFilterString);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);
  json msg_header;
  std::array<zmq::message_t, kNumMessages> msg_parts;
  try {
    while (!RIP) {
      auto it = msg_parts.begin();
      auto recv_result = zmq::recv_multipart_n(sock, it, kNumMessages);
      // TODO: report empty recv_result under verbose
      if (recv_result) {
        msg_header = json::parse(msg_parts[1].to_string());
        std::cout << msg_header["spike"].dump() << std::endl;
        /* Tentative method for detecting a motor command:
         * Every time a spike is received, a time difference value is calculated
         * by subtracting its timestamp from that of its immediate predecessor.
         * This value is compared against a set threshold. If it exceeds the
         * threshold, a cumulative score is incremented. Once this score reach a
         * set level, there is a motor command. If the time difference falls
         * below the threshold, the score collapses to zero.
         */
      } else {
        LogText("TIMEOUT");
      }
    }
  } catch (std::exception &e) {
    std::cout << "Exception while receiving data: " << e.what() << std::endl;
    RIP = 1;
    return;
  }
}
