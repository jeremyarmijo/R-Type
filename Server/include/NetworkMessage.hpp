#pragma once
#include <cstdint>
#include <vector>

struct NetworkMessage {
  uint32_t client_id;
  std::vector<uint8_t> data;
  uint64_t timestamp;

  NetworkMessage() : client_id(0), timestamp(0) {}
};
