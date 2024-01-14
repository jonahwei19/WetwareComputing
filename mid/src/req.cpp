#include "req.hpp"
#include "signal_handler.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <json/json.h>
#include <uuid/uuid.h>
#include <zmq.hpp>

std::string GenerateUuid() {
  uuid_t uuid_unparsed;
  char uuid_parsed[UUID_STR_LEN];
  uuid_generate_random(uuid_unparsed);
  uuid_unparse_upper(uuid_unparsed, uuid_parsed);
  return uuid_parsed;
}

std::string GenerateHeartbeat(const std::string uuid) {
  Json::Value object(Json::objectValue);
  object["application"] = "mid";
  object["type"] = "heartbeat";
  object["uuid"] = uuid;
  return object.toStyledString();
}

void RequestLoop(const std::string addr, zmq::context_t &ctx,
                 const bool is_verbose) {
  const auto kAppUuid = GenerateUuid();
  const auto kHeartbeatString = GenerateHeartbeat(kAppUuid);
  auto send_buf = zmq::buffer(kHeartbeatString);
  std::string recv_buf_str;
  auto recv_buf = zmq::buffer(recv_buf_str);
  zmq::socket_t sock(ctx, zmq::socket_type::req);
  sock.set(zmq::sockopt::linger, 0);
  sock.set(zmq::sockopt::rcvtimeo, 2000);
  sock.connect(addr);

  try {
    while (!RIP) {
      sock.send(send_buf);
      auto recv_buffer_result = sock.recv(recv_buf);
      // TODO: report empty recv_buffer_result under verbose
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  } catch (std::exception &e) {
    std::cout << "Heartbeat failure encountered: " << e.what() << std::endl;
    RIP = 1;
    return;
  }
}
