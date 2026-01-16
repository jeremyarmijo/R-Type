#include "network/DecodeFunc.hpp"

#include "network/DataMask.hpp"

// #include <arpa/inet.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

bool checkHeader(const std::vector<uint8_t>& packet, size_t& offset,
                 uint32_t& payloadLength, uint16_t& seq, uint16_t& ack,
                 uint32_t& ack_bits) {
  if (packet.size() < 6) return false;

  uint8_t flags = packet[1];

  memcpy(&payloadLength, &packet[2], 4);
  payloadLength = ntohl(payloadLength);

  offset = 6;

  if ((flags & 0x02) || (flags & 0x08)) {
    if (packet.size() < 14) return false;
    memcpy(&seq, &packet[offset], 2);
    seq = ntohs(seq);
    offset += 2;

    memcpy(&ack, &packet[offset], 2);
    ack = ntohs(ack);
    offset += 2;

    memcpy(&ack_bits, &packet[offset], 4);
    ack_bits = ntohl(ack_bits);
    offset += 4;
  }
  return packet.size() == (offset + payloadLength);
}

Event DecodeLOGIN_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_REQUEST;
  LOGIN_REQUEST data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  uint8_t usernameLen = packet[offset++];
  data.username =
      std::string(reinterpret_cast<const char*>(&packet[offset]), usernameLen);
  offset += usernameLen;

  uint8_t passwordLen = packet[offset++];
  data.password =
      std::string(reinterpret_cast<const char*>(&packet[offset]), passwordLen);
  offset += passwordLen;

  evt.data = data;
  return evt;
}

Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_RESPONSE;

  LOGIN_RESPONSE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.success = packet[offset++];
  if (data.success == 1) {
    memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
    data.playerId = ntohs(data.playerId);
    offset += sizeof(data.playerId);

    memcpy(&data.udpPort, &packet[offset], sizeof(data.udpPort));
    data.udpPort = ntohs(data.udpPort);
    offset += sizeof(data.udpPort);
  } else {
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    data.errorCode = ntohs(data.errorCode);
    offset += sizeof(data.errorCode);

    uint8_t msgLen = packet[offset++];
    data.message =
        std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
    offset += msgLen;
  }

  evt.data = data;
  return evt;
}

Event DecodeGAME_START(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_START;

  GAME_START data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  uint32_t temp;
  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.playerSpawnX, &temp, sizeof(data.playerSpawnX));
  offset += sizeof(temp);

  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.playerSpawnY, &temp, sizeof(data.playerSpawnY));
  offset += sizeof(temp);

  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.scrollSpeed, &temp, sizeof(data.scrollSpeed));
  offset += sizeof(temp);

  evt.data = data;
  return evt;
}

Event DecodeGAME_END(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_END;

  GAME_END data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.victory = packet[offset++];
  uint8_t playerCount = packet[offset++];
  data.scores.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    GAME_END::Score score;
    memcpy(&score.playerId, &packet[offset], sizeof(score.playerId));
    score.playerId = ntohs(score.playerId);
    offset += sizeof(score.playerId);

    memcpy(&score.score, &packet[offset], sizeof(score.score));
    score.score = ntohl(score.score);
    offset += sizeof(score.score);

    score.rank = packet[offset++];
    data.scores.push_back(score);
  }

  evt.data = data;
  return evt;
}

Event DecodeERROR(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ERROR_TYPE;

  ERROR_EVNT data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
  data.errorCode = ntohs(data.errorCode);
  offset += sizeof(data.errorCode);

  uint8_t msgLen = packet[offset++];
  data.message =
      std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodePLAYER_INPUT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_INPUT;

  PLAYER_INPUT data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.up = packet[offset++] == 1;
  data.down = packet[offset++] == 1;
  data.left = packet[offset++] == 1;
  data.right = packet[offset++] == 1;
  data.fire = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeGAME_STATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_STATE;
  GAME_STATE data;
  size_t offset = 0;
  uint32_t payloadLength;
  uint16_t seq, ack;
  uint32_t ack_bits;
  uint32_t tempFloat;

  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits)) {
    return evt;
  }

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  // JOUEURS
  if (offset < packet.size()) {
    uint8_t numPlayers = packet[offset++];
    for (int i = 0; i < numPlayers; ++i) {
      if (offset + 4 > packet.size()) break;

      GAME_STATE::PlayerState p;

      memcpy(&p.playerId, &packet[offset], 2);
      p.playerId = ntohs(p.playerId);
      offset += 2;

      memcpy(&p.mask, &packet[offset], 2);
      p.mask = ntohs(p.mask);
      offset += 2;

      if (p.mask & M_POS_X) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&p.posX, &tempFloat, 4);
        offset += 4;
      }
      if (p.mask & M_POS_Y) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&p.posY, &tempFloat, 4);
        offset += 4;
      }
      if (p.mask & M_HP) {
        if (offset >= packet.size()) break;
        p.hp = packet[offset++];
      }
      if (p.mask & M_STATE) {
        if (offset >= packet.size()) break;
        p.state = packet[offset++];
      }
      if (p.mask & M_SHIELD) {
        if (offset >= packet.size()) break;
        p.shield = packet[offset++];
      }
      if (p.mask & M_WEAPON) {
        if (offset >= packet.size()) break;
        p.weapon = packet[offset++];
      }
      if (p.mask & M_SPRITE) {
        if (offset >= packet.size()) break;
        p.sprite = packet[offset++];
      }

      data.players.push_back(p);
    }
  }

  // ENNEMIS
  if (offset < packet.size()) {
    uint8_t numEnemies = packet[offset++];
    for (int i = 0; i < numEnemies; ++i) {
      if (offset + 4 > packet.size()) break;

      GAME_STATE::EnemyState e;

      memcpy(&e.enemyId, &packet[offset], 2);
      e.enemyId = ntohs(e.enemyId);
      offset += 2;

      memcpy(&e.mask, &packet[offset], 2);
      e.mask = ntohs(e.mask);
      offset += 2;

      if (e.mask & M_POS_X) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&e.posX, &tempFloat, 4);
        offset += 4;
      }
      if (e.mask & M_POS_Y) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&e.posY, &tempFloat, 4);
        offset += 4;
      }
      if (e.mask & M_HP) {
        if (offset >= packet.size()) break;
        e.hp = packet[offset++];
      }
      if (e.mask & M_STATE) {
        if (offset >= packet.size()) break;
        e.state = packet[offset++];
      }
      if (e.mask & M_TYPE) {
        if (offset >= packet.size()) break;
        e.enemyType = packet[offset++];
      }
      if (e.mask & M_DIR) {
        if (offset >= packet.size()) break;
        e.direction = (int8_t)packet[offset++];
      }

      data.enemies.push_back(e);
    }
  }

  // PROJECTILES
  if (offset < packet.size()) {
    uint8_t numProjectiles = packet[offset++];
    for (int i = 0; i < numProjectiles; ++i) {
      if (offset + 4 > packet.size()) break;

      GAME_STATE::ProjectileState pr;

      memcpy(&pr.projectileId, &packet[offset], 2);
      pr.projectileId = ntohs(pr.projectileId);
      offset += 2;

      memcpy(&pr.mask, &packet[offset], 2);
      pr.mask = ntohs(pr.mask);
      offset += 2;

      if (pr.mask & M_POS_X) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&pr.posX, &tempFloat, 4);
        offset += 4;
      }
      if (pr.mask & M_POS_Y) {
        memcpy(&tempFloat, &packet[offset], 4);
        tempFloat = ntohl(tempFloat);
        memcpy(&pr.posY, &tempFloat, 4);
        offset += 4;
      }
      if (pr.mask & M_TYPE) {
        if (offset >= packet.size()) break;
        pr.type = packet[offset++];
      }
      if (pr.mask & M_OWNER) {
        if (offset + 2 > packet.size()) break;
        uint16_t owner;
        memcpy(&owner, &packet[offset], 2);
        pr.ownerId = ntohs(owner);
        offset += 2;
      }
      if (pr.mask & M_DAMAGE) {
        if (offset >= packet.size()) break;
        pr.damage = packet[offset++];
      }

      data.projectiles.push_back(pr);
    }
  }

  evt.data = data;
  return evt;
}

Event DecodeAUTH(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::AUTH;

  AUTH data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  data.playerId = ntohs(data.playerId);
  offset += sizeof(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeBOSS_SPAWN(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::BOSS_SPAWN;

  BOSS_SPAWN data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.bossId, &packet[offset], sizeof(data.bossId));
  data.bossId = ntohs(data.bossId);
  offset += sizeof(data.bossId);

  data.bossType = packet[offset++];

  memcpy(&data.maxHp, &packet[offset], sizeof(data.maxHp));
  data.maxHp = ntohs(data.maxHp);
  offset += sizeof(data.maxHp);

  data.phase = packet[offset++];
  evt.data = data;
  return evt;
}

Event DecodeBOSS_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::BOSS_UPDATE;

  BOSS_UPDATE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.bossId, &packet[offset], sizeof(data.bossId));
  data.bossId = ntohs(data.bossId);
  offset += sizeof(data.bossId);

  uint32_t temp;
  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.posX, &temp, sizeof(data.posX));
  offset += sizeof(temp);

  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.posY, &temp, sizeof(data.posY));
  offset += sizeof(temp);

  memcpy(&data.hp, &packet[offset], sizeof(data.hp));
  data.hp = ntohs(data.hp);
  offset += sizeof(data.hp);

  data.phase = packet[offset++];
  data.action = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeENEMY_HIT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ENEMY_HIT;

  ENEMY_HIT data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.enemyId, &packet[offset], sizeof(data.enemyId));
  data.enemyId = ntohs(data.enemyId);
  offset += sizeof(data.enemyId);

  data.damage = packet[offset++];

  memcpy(&data.hpRemaining, &packet[offset], sizeof(data.hpRemaining));
  data.hpRemaining = ntohs(data.hpRemaining);
  offset += sizeof(data.hpRemaining);

  evt.data = data;
  return evt;
}

Event DecodeFORCE_STATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::FORCE_STATE;

  FORCE_STATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.forceId, &packet[offset], sizeof(data.forceId));
  data.forceId = ntohs(data.forceId);
  offset += sizeof(data.forceId);

  memcpy(&data.ownerId, &packet[offset], sizeof(data.ownerId));
  data.ownerId = ntohs(data.ownerId);
  offset += sizeof(data.ownerId);

  uint32_t temp;
  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.posX, &temp, sizeof(data.posX));
  offset += sizeof(temp);

  memcpy(&temp, &packet[offset], sizeof(temp));
  temp = ntohl(temp);
  memcpy(&data.posY, &temp, sizeof(data.posY));
  offset += sizeof(temp);

  data.state = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_CREATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_CREATE;

  LOBBY_CREATE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  uint8_t lobbyNameLen = packet[offset++];
  data.lobbyName =
      std::string(reinterpret_cast<const char*>(&packet[offset]), lobbyNameLen);
  offset += lobbyNameLen;

  uint8_t playerNameLen = packet[offset++];
  data.playerName = std::string(reinterpret_cast<const char*>(&packet[offset]),
                                playerNameLen);
  offset += playerNameLen;

  uint8_t passwordLen = packet[offset++];
  data.password =
      std::string(reinterpret_cast<const char*>(&packet[offset]), passwordLen);
  offset += passwordLen;

  data.Maxplayer = packet[offset++];
  data.difficulty = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_JOIN_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_JOIN_REQUEST;
  LOBBY_JOIN_REQUEST data;

  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  if (packet.size() < offset + 2) return evt;

  uint16_t rawId;
  memcpy(&rawId, &packet[offset], 2);
  data.lobbyId = ntohs(rawId);
  offset += 2;

  if (packet.size() < offset + 1) return evt;
  uint8_t nameLen = packet[offset++];
  if (nameLen > 0 && (offset + nameLen <= packet.size())) {
    data.name =
        std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;
  }

  if (packet.size() < offset + 1) return evt;
  uint8_t passwordLen = packet[offset++];
  if (passwordLen > 0 && (offset + passwordLen <= packet.size())) {
    data.password = std::string(reinterpret_cast<const char*>(&packet[offset]),
                                passwordLen);
    offset += passwordLen;
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_JOIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_JOIN_RESPONSE;

  LOBBY_JOIN_RESPONSE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.success = packet[offset++];

  if (data.success == 1) {
    memcpy(&data.lobbyId, &packet[offset], sizeof(data.lobbyId));
    data.lobbyId = ntohs(data.lobbyId);
    offset += sizeof(data.lobbyId);

    memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
    data.playerId = ntohs(data.playerId);
    offset += sizeof(data.playerId);

    uint8_t playerCount = packet[offset++];
    data.players.reserve(playerCount);

    for (uint8_t i = 0; i < playerCount; ++i) {
      LOBBY_JOIN_RESPONSE::Player player;

      memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
      player.playerId = ntohs(player.playerId);
      offset += sizeof(player.playerId);

      player.ready = packet[offset++] == 1;

      uint8_t usernameLen = packet[offset++];
      player.username = std::string(
          reinterpret_cast<const char*>(&packet[offset]), usernameLen);
      offset += usernameLen;

      data.players.push_back(player);
    }
  } else {
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    data.errorCode = ntohs(data.errorCode);
    offset += sizeof(data.errorCode);

    uint8_t msgLen = packet[offset++];
    data.errorMessage =
        std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
    offset += msgLen;
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LIST_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LIST_RESPONSE;

  LOBBY_LIST_RESPONSE data;
  size_t offset = 0;
  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  uint8_t lobbyCount = packet[offset++];
  data.lobbies.reserve(lobbyCount);

  for (uint8_t i = 0; i < lobbyCount; ++i) {
    Lobbies lobby;

    memcpy(&lobby.lobbyId, &packet[offset], sizeof(lobby.lobbyId));
    lobby.lobbyId = ntohs(lobby.lobbyId);
    offset += sizeof(lobby.lobbyId);

    uint8_t nameLen = packet[offset++];
    lobby.name =
        std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;

    lobby.playerCount = packet[offset++];
    lobby.maxPlayers = packet[offset++];
    lobby.difficulty = packet[offset++];
    lobby.isStarted = (packet[offset++] == 1);
    lobby.hasPassword = (packet[offset++] == 1);

    data.lobbies.push_back(lobby);
  }

  evt.data = data;
  return evt;
}

Event DecodePLAYER_READY(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_READY;

  PLAYER_READY data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.ready = packet[offset++] == 1;

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_UPDATE;

  LOBBY_UPDATE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  uint8_t nameLen = packet[offset++];
  data.name =
      std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
  offset += nameLen;

  memcpy(&data.hostId, &packet[offset], sizeof(data.hostId));
  data.hostId = ntohs(data.hostId);
  offset += sizeof(data.hostId);

  data.asStarted = (packet[offset++] == 1);
  data.maxPlayers = packet[offset++];
  data.difficulty = packet[offset++];

  uint8_t playerCount = packet[offset++];
  data.playerInfo.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    PlayerInfo player;

    memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
    player.playerId = ntohs(player.playerId);
    offset += sizeof(player.playerId);

    player.ready = (packet[offset++] == 1);

    uint8_t usernameLen = packet[offset++];
    player.username = std::string(
        reinterpret_cast<const char*>(&packet[offset]), usernameLen);
    offset += usernameLen;

    data.playerInfo.push_back(player);
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_KICK(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_KICK;

  LOBBY_KICK data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  data.playerId = ntohs(data.playerId);
  offset += sizeof(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_START(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_START;

  LOBBY_START data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  data.countdown = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LIST_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LIST_REQUEST;
  LOBBY_LIST_REQUEST data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.playerId, &packet[offset], 2);
  data.playerId = ntohs(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LEAVE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LEAVE;
  LOBBY_LEAVE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.playerId, &packet[offset], 2);
  data.playerId = ntohs(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeMESSAGE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::MESSAGE;

  MESSAGE data;
  size_t offset = 0;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.lobbyId, &packet[offset], sizeof(data.lobbyId));
  data.lobbyId = ntohs(data.lobbyId);
  offset += sizeof(data.lobbyId);

  uint8_t nameLen = packet[offset++];
  data.playerName =
      std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
  offset += nameLen;

  uint8_t msgLen = packet[offset++];
  data.message =
      std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodeCLIENT_LEAVE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CLIENT_LEAVE;

  CLIENT_LEAVE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  uint16_t seq = 0;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;
  if (!checkHeader(packet, offset, payloadLength, seq, ack, ack_bits))
    return Event{};

  evt.seqNum = seq;
  evt.ack = ack;
  evt.ack_bits = ack_bits;

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  data.playerId = ntohs(data.playerId);
  offset += sizeof(data.playerId);

  evt.data = data;
  return evt;
}


void SetupDecoder(Decoder& decoder) {
  // TCP Messages
  decoder.registerHandler(0x01, DecodeLOGIN_REQUEST);
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
  decoder.registerHandler(0x0F, DecodeGAME_START);
  decoder.registerHandler(0x10, DecodeGAME_END);
  decoder.registerHandler(0x11, DecodeCLIENT_LEAVE);
  decoder.registerHandler(0x12, DecodeERROR);

  decoder.registerHandler(0x03, DecodeLOBBY_CREATE);
  decoder.registerHandler(0x04, DecodeLOBBY_JOIN_REQUEST);
  decoder.registerHandler(0x05, DecodeLOBBY_JOIN_RESPONSE);
  decoder.registerHandler(0x06, DecodeLOBBY_LIST_REQUEST);
  decoder.registerHandler(0x07, DecodeLOBBY_LIST_RESPONSE);
  decoder.registerHandler(0x08, DecodePLAYER_READY);
  decoder.registerHandler(0x09, DecodeLOBBY_UPDATE);
  decoder.registerHandler(0x0A, DecodeLOBBY_LEAVE);
  decoder.registerHandler(0x0B, DecodeLOBBY_START);
  decoder.registerHandler(0x0C, DecodeMESSAGE);
  decoder.registerHandler(0x0D, DecodeLOBBY_KICK);

  // UDP Messages
  decoder.registerHandler(0x20, DecodePLAYER_INPUT);
  decoder.registerHandler(0x21, DecodeGAME_STATE);
  decoder.registerHandler(0x22, DecodeAUTH);
  decoder.registerHandler(0x23, DecodeBOSS_SPAWN);
  decoder.registerHandler(0x24, DecodeBOSS_UPDATE);
  decoder.registerHandler(0x25, DecodeENEMY_HIT);
  decoder.registerHandler(0x26, DecodeFORCE_STATE);
}
