#include "sub.hpp"
#include "signal_handler.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose) {
  using nlohmann::json;
  constexpr auto kNumMessages = 3;
  constexpr auto kFilterString{"EVENT"};
  zmq::socket_t sock(ctx, zmq::socket_type::sub);
  sock.set(zmq::sockopt::subscribe, kFilterString);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);
  json header_message;
  std::array<zmq::message_t, kNumMessages> messages;
  try {
    while (!RIP) {
      auto it = messages.begin();
      auto recv_result = zmq::recv_multipart_n(sock, it, kNumMessages);
      // TODO: report empty recv_result under verbose
      if (recv_result) {
        header_message = json::parse(messages[1].to_string());
        std::cout << header_message.dump() << std::endl;
      }
    }
  } catch (std::exception &e) {
    std::cout << "Exception while receiving data: " << e.what() << std::endl;
    RIP = 1;
    return;
  }
}
