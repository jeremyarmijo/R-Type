#pragma once

#include <cstdint>
#include <vector>
#include <asio.hpp>


using UDPEndpoint = asio::ip::udp::endpoint;
using ErrorCode = asio::error_code;

struct NetworkMessage {
  uint32_t client_id;  // 0 pour serveur
  std::vector<uint8_t> data;
  uint64_t timestamp;  // Pour latence/ordering

  NetworkMessage() : client_id(0), timestamp(0) {}
};
