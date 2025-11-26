#pragma once
#include <vector>

#include "Decoder.hpp"
#include "Event.hpp"

// Handlers R-Type

Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  LOGIN_RESPONSE data;
  evt.type = EventType::LOGIN_RESPONSE;
  // data.playerId = ...
  evt.data = data;
  return evt;
}

void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x01, DecodeLOGIN_RESPONSE);
}
