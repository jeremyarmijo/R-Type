#pragma once
#include <cstdint>
#include <vector>

struct NetworkMessage {
  uint16_t client_id;
  std::vector<uint8_t> data;

  NetworkMessage() : client_id(0) {}
};
