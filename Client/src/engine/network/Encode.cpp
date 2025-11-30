#include <vector>

#include "include/Encode.hpp"

Encoder::Encoder() { handlers.fill(nullptr); }

void Encoder::registerHandler(ActionType type, EncodePayload f) {
  handlers[static_cast<uint8_t>(type)] = f;
}

std::vector<uint8_t> Encoder::encode(const Action& a, size_t useUDP,
                                     uint32_t sequenceNum) {
  auto& func = handlers[static_cast<uint8_t>(a.type)];
  if (!func) return {};

  std::vector<uint8_t> payload;
  payload.reserve(64);
  func(a, payload, sequenceNum);

  std::vector<uint8_t> packet;
  packet.reserve(6 + payload.size());

  PacketHeader header;
  header.type = static_cast<uint8_t>(a.type);
  header.flags = 0;

  if (useUDP == 0) header.flags |= 0x02;
  if (useUDP == 1) header.flags |= 0x08;
  if (useUDP == 2) header.flags |= 0x01;

  header.length = payload.size();
  writeHeader(packet, header);
  packet.insert(packet.end(), payload.begin(), payload.end());
  return packet;
}

void Encoder::writeHeader(std::vector<uint8_t>& packet, const PacketHeader& h) {
  packet.push_back(h.type);
  packet.push_back(h.flags);

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&h.length);
  packet.insert(packet.end(), ptr, ptr + 4);
}
