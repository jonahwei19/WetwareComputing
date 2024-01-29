#include "req.hpp"
#include "signal_handler.hpp"
#include "sub.hpp"
#include "time_utils.hpp"

#include <iostream>
#include <string>
#include <thread>

#include <zmq.hpp>

#include "CLI11/CLI11.hpp"

int main(int argc, char **argv) {
  signal_handler_setup();
  bool is_verbose = false;
  int port_number = 5556;
  JuceTime maximum_interval = 500;
  int number_of_spikes = 5;
  CLI::App app{"the middle process, incorporating game logic and interprocess "
               "automation"};
  app.add_flag("-v", is_verbose, "enable verbose output");
  app.add_option("-p", port_number, "zmq_interface port number (pub/sub)")
      ->capture_default_str();
  app.add_option("-t", maximum_interval,
                 "maximum time gap between two spikes for them to be "
                 "considered consecutive (ms)")
      ->capture_default_str();
  app.add_option(
         "-n", number_of_spikes,
         "number of consecutive spikes required to trigger a motor event")
      ->capture_default_str();
  CLI11_PARSE(app, argc, argv);
  constexpr auto kAddrPrefix{"tcp://localhost:"};
  const auto kReqAddr = kAddrPrefix + std::to_string(port_number + 1);
  const auto kSubAddr = kAddrPrefix + std::to_string(port_number);
  std::cout << "req (heartbeat) addr: " << kReqAddr
            << "\nsub addr: " << kSubAddr << std::endl;
  TriggerConditions trigger_conditions{maximum_interval, number_of_spikes};
  zmq::context_t ctx;
  std::thread req_thread(RequestLoop, kReqAddr, std::ref(ctx), is_verbose);
  LogText("Created RequestLoop thread");
  std::thread sub_thread(SubscribeLoop, kSubAddr, std::ref(ctx), is_verbose,
                         trigger_conditions);
  LogText("Created SubscribeLoop thread");
  req_thread.join();
  LogText("Exited RequestLoop thread");
  sub_thread.join();
  LogText("Exited SubscribeLoop thread");
  LogText("Graceful Exit");
  return 0;
}
