#pragma once
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "Decoder.hpp"
#include "Event.hpp"

Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_RESPONSE;

  LOGIN_RESPONSE data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];

  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  data.success = packet[offset++];

  if (data.success == 1) {
    memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
    offset += sizeof(data.playerId);

    memcpy(&data.serverTick, &packet[offset], sizeof(data.serverTick));
    offset += sizeof(data.serverTick);

    memcpy(&data.udpPort, &packet[offset], sizeof(data.udpPort));
    offset += sizeof(data.udpPort);
  } else {
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    offset += sizeof(data.errorCode);

    uint8_t msgLen = packet[offset++];
    data.message =
        std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
    offset += msgLen;
  }

  evt.data = data;
  return evt;
}

void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
}
