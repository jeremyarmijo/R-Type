#include "include/Encode.hpp"

#include <vector>

Encoder::Encoder() { handlers.fill(nullptr); }

void Encoder::registerHandler(ActionType type, EncodePayload f) {
  handlers[static_cast<uint8_t>(type)] = f;
}

inline uint8_t getType(const Action& a) {
    switch (a.type) {
        case ActionType::LOGIN_REQUEST:      return 0x01;
        case ActionType::SIGNUP_REQUEST:     return 0x03;
        case ActionType::LOGOUT:             return 0x05;
        case ActionType::LOBBY_LIST_REQUEST: return 0x06;
        case ActionType::LOBBY_JOIN:         return 0x08;
        case ActionType::PLAYER_READY:       return 0x0A;
        case ActionType::LOBBY_LEAVE:        return 0x0B;
        case ActionType::PLAYER_END_LOADING: return 0x0E;
        case ActionType::CHUNK_REQUEST:      return 0x13;

        case ActionType::AUTH:                return 0x19;
        case ActionType::UP:                  return 0x20;
        case ActionType::DOWN:                return 0x20;
        case ActionType::LEFT:                return 0x20;
        case ActionType::RIGHT:               return 0x20;
        case ActionType::SHOOT:               return 0x20;
        case ActionType::FORCE_ATTACH:        return 0x20;
        case ActionType::FORCE_DETACH:        return 0x20;
        case ActionType::USE_POWERUP:         return 0x20;

        default: return 0xFF;
    }
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
  header.type = getType(a);
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

    uint32_t length = htonl(h.length);
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&length);
    packet.insert(packet.end(), ptr, ptr + 4);
}
