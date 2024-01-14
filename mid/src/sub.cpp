#include "sub.hpp"
#include "signal_handler.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <zmq.hpp>
#include <zmq_addon.hpp>

void SubscribeLoop(const std::string addr, zmq::context_t &ctx,
                   const bool is_verbose) {
  constexpr auto kNumMessages = 3;
  constexpr auto kFilterString{"EVENT"};
  zmq::socket_t sock(ctx, zmq::socket_type::sub);
  sock.set(zmq::sockopt::subscribe, kFilterString);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);
  std::array<zmq::message_t, kNumMessages> messages;
  try {
    while (!RIP) {
      auto it = messages.begin();
      auto recv_result = zmq::recv_multipart_n(sock, it, kNumMessages);
      // TODO: report empty recv_result under verbose
      if (recv_result) {
        for (const auto &message : messages) {
          std::cout << message << std::endl;
        }
      }
    }
  } catch (std::exception &e) {
    std::cout << "Exception while receiving data: " << e.what() << std::endl;
    RIP = 1;
    return;
  }
}
