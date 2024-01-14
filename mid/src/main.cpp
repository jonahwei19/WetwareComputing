#include "req.hpp"
#include "signal_handler.hpp"
#include "sub.hpp"

#include <iostream>
#include <string>
#include <thread>

#include <zmq.hpp>

#include "CLI11/CLI11.hpp"

int main(int argc, char **argv) {
  signal_handler_setup();
  bool is_verbose = false;
  int port_number = 5556;
  CLI::App app{"the middle process, incorporating game logic and interprocess "
               "automation"};
  app.add_flag("-v", is_verbose, "enable verbose output");
  app.add_option("-p", port_number, "zmq_interface port number (pub/sub)")
      ->capture_default_str();
  CLI11_PARSE(app, argc, argv);
  constexpr auto kAddrPrefix{"tcp://localhost:"};
  const auto kReqAddr = kAddrPrefix + std::to_string(port_number + 1);
  const auto kSubAddr = kAddrPrefix + std::to_string(port_number);
  std::cout << "req (heartbeat) addr: " << kReqAddr
            << "\nsub addr: " << kSubAddr << std::endl;
  zmq::context_t ctx;
  std::thread req_thread(RequestLoop, kReqAddr, std::ref(ctx), is_verbose);
  std::thread sub_thread(SubscribeLoop, kSubAddr, std::ref(ctx), is_verbose);
  req_thread.join();
  sub_thread.join();
  std::cout << "Graceful Exit" << std::endl;
  return 0;
}
