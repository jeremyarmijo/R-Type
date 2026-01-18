// #include <arpa/inet.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

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
    case ActionType::LOBBY_CREATE:
      return 0x03;
    case ActionType::LOBBY_JOIN_REQUEST:
      return 0x04;
    case ActionType::LOBBY_JOIN_RESPONSE:
      return 0x05;
    case ActionType::LOBBY_LIST_REQUEST:
      return 0x06;
    case ActionType::LOBBY_LIST_RESPONSE:
      return 0x07;
    case ActionType::PLAYER_READY:
      return 0x08;
    case ActionType::LOBBY_UPDATE:
      return 0x09;
    case ActionType::LOBBY_LEAVE:
      return 0x0A;
    case ActionType::LOBBY_START:
      return 0x0B;
    case ActionType::MESSAGE:
      return 0x0C;
    case ActionType::LOBBY_KICK:
      return 0x0D;
    case ActionType::SEND_MAP:
      return 0x0E;
    case ActionType::GAME_START:
      return 0x0F;
    case ActionType::GAME_END:
      return 0x10;
    case ActionType::CLIENT_LEAVE:
      return 0x11;
    case ActionType::ERROR_SERVER:
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
      return 0x20;

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
    case ActionType::FORCE_STATE:
      return 0x26;

    default:
      return 0xFF;
  }
}

std::vector<uint8_t> Encoder::encode(const Action& a, size_t useUDP,
                                     uint16_t seqNum, uint16_t ack,
                                     uint32_t ack_bytes) {
  auto& func = handlers[static_cast<uint8_t>(a.type)];
  if (!func) return {};

  std::vector<uint8_t> payload;
  payload.reserve(64);
  func(a, payload);

  size_t headerSize = (useUDP == 0 || useUDP == 1) ? 14 : 6;

  std::vector<uint8_t> packet;
  packet.reserve(headerSize + payload.size());

  PacketHeader header;
  header.type = getType(a);
  header.flags = 0;

  if (useUDP == 0) header.flags |= 0x02;
  if (useUDP == 1) header.flags |= 0x08;
  if (useUDP == 2) header.flags |= 0x01;

  header.length = static_cast<uint32_t>(payload.size());
  header.seqNum = seqNum;
  header.ack = ack;
  header.ack_bytes = ack_bytes;

  writeHeader(packet, header);
  packet.insert(packet.end(), payload.begin(), payload.end());

  return packet;
}

void Encoder::writeHeader(std::vector<uint8_t>& packet, const PacketHeader& h) {
  packet.push_back(h.type);
  packet.push_back(h.flags);

  uint32_t length = htonl(h.length);
  const uint8_t* pLen = reinterpret_cast<const uint8_t*>(&length);
  packet.insert(packet.end(), pLen, pLen + 4);

  if ((h.flags & 0x02) || (h.flags & 0x08)) {
    uint16_t s = htons(h.seqNum);
    const uint8_t* pSeq = reinterpret_cast<const uint8_t*>(&s);
    packet.insert(packet.end(), pSeq, pSeq + 2);

    uint16_t a = htons(h.ack);
    const uint8_t* pAck = reinterpret_cast<const uint8_t*>(&a);
    packet.insert(packet.end(), pAck, pAck + 2);

    uint32_t ab = htonl(h.ack_bytes);
    const uint8_t* pAb = reinterpret_cast<const uint8_t*>(&ab);
    packet.insert(packet.end(), pAb, pAb + 4);
  }
}
