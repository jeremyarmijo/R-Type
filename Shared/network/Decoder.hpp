#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>
#include "network/Event.hpp"

class Decoder {
 public:
  using DecodeFunc = std::function<Event(const std::vector<uint8_t>&)>;

  Decoder();

  void registerHandler(uint8_t packetType, DecodeFunc func);
  Event decode(const std::vector<uint8_t>& packet);

 private:
  std::array<DecodeFunc, 256> handlers;
};
