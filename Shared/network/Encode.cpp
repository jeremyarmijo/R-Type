#include <arpa/inet.h>

#include <vector>

#include "network/Encoder.hpp"

Encoder::Encoder() { handlers.fill(nullptr); }

void Encoder::registerHandler(ActionType type, EncodePayload f) {
  handlers[static_cast<uint8_t>(type)] = f;
}

inline uint8_t getType(const Action& a) {
  switch (a.type) {
    case ActionType::LOGIN_REQUEST:
      return 0x01;
    case ActionType::LOGIN_RESPONSE:
      return 0x02;
    case ActionType::GAME_START:
      return 0x0F;
    case ActionType::GAME_END:
      return 0x10;
    case ActionType::ERROR:
      return 0x12;

    case ActionType::UP_PRESS:
    case ActionType::UP_RELEASE:
    case ActionType::DOWN_PRESS:
    case ActionType::DOWN_RELEASE:
    case ActionType::LEFT_PRESS:
    case ActionType::LEFT_RELEASE:
    case ActionType::RIGHT_PRESS:
    case ActionType::RIGHT_RELEASE:
    case ActionType::FIRE_PRESS:
    case ActionType::FIRE_RELEASE:
      return 0x20;  // PLAYER_INPUT

    case ActionType::GAME_STATE:
      return 0x21;
    case ActionType::AUTH:
      return 0x22;
    case ActionType::BOSS_SPAWN:
      return 0x23;
    case ActionType::BOSS_UPDATE:
      return 0x24;
    case ActionType::ENEMY_HIT:
      return 0x25;

    default:
      return 0xFF;
  }
}

std::vector<uint8_t> Encoder::encode(const Action& a, size_t useUDP) {
  auto& func = handlers[static_cast<uint8_t>(a.type)];
  if (!func) return {};

  std::vector<uint8_t> payload;
  payload.reserve(64);
  func(a, payload);

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
