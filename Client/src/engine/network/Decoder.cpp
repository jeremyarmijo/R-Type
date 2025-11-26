#include <vector>

#include "network/Decoder.hpp"

Decoder::Decoder() { handlers.fill(nullptr); }

void Decoder::registerHandler(uint8_t packetType, DecodeFunc func) {
  handlers[packetType] = func;
}

Event Decoder::decode(const std::vector<uint8_t>& packet) {
  if (packet.empty()) {
    return Event{};
  }

  uint8_t type = packet[0];

  if (!handlers[type]) {
    return Event{};
  }
  return handlers[type](packet);
}
