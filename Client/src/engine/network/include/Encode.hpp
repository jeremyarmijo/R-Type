#pragma once

#include <arpa/inet.h>

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

#include "Action.hpp"

struct PacketHeader {
  uint8_t type;
  uint8_t flags;
  uint32_t length;
};

class Encoder {
 public:
  using EncodePayload =
      std::function<void(const Action&, std::vector<uint8_t>&, uint32_t)>;

  Encoder();
  void registerHandler(ActionType type, EncodePayload f);
  std::vector<uint8_t> encode(const Action& a, size_t useUDP,
                              uint32_t sequenceNum);

 private:
  std::array<EncodePayload, 256> handlers;
  void writeHeader(std::vector<uint8_t>& packet, const PacketHeader& h);
};
