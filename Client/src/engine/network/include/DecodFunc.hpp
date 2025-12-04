#pragma once
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "Decoder.hpp"
#include "Event.hpp"

bool checkHeader(const std::vector<uint8_t>& packet, size_t& offset,
                 uint32_t& payloadLength) {
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return false;
  }
  return true;
}

Event DecodeLOGIN_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_REQUEST;

  LOGIN_REQUEST data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
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

Event DecodeSIGNUP_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::SIGNUP_REQUEST;

  SIGNUP_REQUEST data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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

Event DecodeSIGNUP_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::SIGNUP_RESPONSE;

  SIGNUP_RESPONSE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  data.success = packet[offset++];

  if (data.success == 1) {
    uint8_t messageLen = packet[offset++];
    data.message =
        std::string(reinterpret_cast<const char*>(&packet[offset]), messageLen);
    offset += messageLen;
  } else {
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    offset += sizeof(data.errorCode);

    uint8_t messageLen = packet[offset++];
    data.message =
        std::string(reinterpret_cast<const char*>(&packet[offset]), messageLen);
    offset += messageLen;
  }

  evt.data = data;
  return evt;
}

Event DecodeLOGOUT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGOUT;

  LOGOUT data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LIST_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LIST_REQUEST;
  evt.data = LOBBY_LIST_REQUEST{};
  return evt;
}

Event DecodeLOBBY_LIST_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LIST_RESPONSE;

  LOBBY_LIST_RESPONSE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  uint8_t lobbyCount = packet[offset++];
  data.lobbies.reserve(lobbyCount);

  for (uint8_t i = 0; i < lobbyCount; ++i) {
    LOBBY_LIST_RESPONSE::Lobby lobby;
    memcpy(&lobby.lobbyId, &packet[offset], sizeof(lobby.lobbyId));
    offset += sizeof(lobby.lobbyId);

    uint8_t nameLen = packet[offset++];
    lobby.name =
        std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;

    lobby.playerCount = packet[offset++];
    lobby.maxPlayers = packet[offset++];
    lobby.hasStarted = packet[offset++];

    data.lobbies.push_back(lobby);
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_JOIN(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_JOIN;

  LOBBY_JOIN data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.lobbyId, &packet[offset], sizeof(data.lobbyId));
  offset += sizeof(data.lobbyId);

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_UPDATE;

  LOBBY_UPDATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.lobbyId, &packet[offset], sizeof(data.lobbyId));
  offset += sizeof(data.lobbyId);

  uint8_t playerCount = packet[offset++];
  data.players.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    LOBBY_UPDATE::PlayerInfo player;
    memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
    offset += sizeof(player.playerId);

    uint8_t nameLen = packet[offset++];
    player.name =
        std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;

    player.ready = packet[offset++];
    data.players.push_back(player);
  }

  evt.data = data;
  return evt;
}

Event DecodePLAYER_READY(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_READY;

  PLAYER_READY data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  data.ready = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LEAVE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LEAVE;

  LOBBY_LEAVE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  evt.data = data;
  return evt;
}

Event DecodeCHAT_MESSAGE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHAT_MESSAGE;

  CHAT_MESSAGE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.senderId, &packet[offset], sizeof(data.senderId));
  offset += sizeof(data.senderId);

  uint16_t msgLen;
  memcpy(&msgLen, &packet[offset], sizeof(msgLen));
  offset += sizeof(msgLen);

  data.message =
      std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodeGAME_LOADING(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_LOADING;

  GAME_LOADING data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.mapId, &packet[offset], sizeof(data.mapId));
  offset += sizeof(data.mapId);

  data.gameMode = packet[offset++];
  data.difficulty = packet[offset++];

  memcpy(&data.mapWidth, &packet[offset], sizeof(data.mapWidth));
  offset += sizeof(data.mapWidth);

  memcpy(&data.mapHeight, &packet[offset], sizeof(data.mapHeight));
  offset += sizeof(data.mapHeight);

  memcpy(&data.chunkSize, &packet[offset], sizeof(data.chunkSize));
  offset += sizeof(data.chunkSize);

  memcpy(&data.tickStart, &packet[offset], sizeof(data.tickStart));
  offset += sizeof(data.tickStart);

  evt.data = data;
  return evt;
}

Event DecodePLAYER_END_LOADING(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_END_LOADING;

  PLAYER_END_LOADING data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  memcpy(&data.missingChunks, &packet[offset], sizeof(data.missingChunks));
  offset += sizeof(data.missingChunks);

  evt.data = data;
  return evt;
}

Event DecodeGAME_START(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_START;

  GAME_START data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.startTick, &packet[offset], sizeof(data.startTick));
  offset += sizeof(data.startTick);

  memcpy(&data.playerSpawnX, &packet[offset], sizeof(data.playerSpawnX));
  offset += sizeof(data.playerSpawnX);

  memcpy(&data.playerSpawnY, &packet[offset], sizeof(data.playerSpawnY));
  offset += sizeof(data.playerSpawnY);

  memcpy(&data.scrollSpeed, &packet[offset], sizeof(data.scrollSpeed));
  offset += sizeof(data.scrollSpeed);

  evt.data = data;
  return evt;
}

Event DecodeGAME_END(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_END;

  GAME_END data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  data.victory = packet[offset++];
  uint8_t playerCount = packet[offset++];
  data.scores.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    GAME_END::Score score;
    memcpy(&score.playerId, &packet[offset], sizeof(score.playerId));
    offset += sizeof(score.playerId);

    memcpy(&score.score, &packet[offset], sizeof(score.score));
    offset += sizeof(score.score);

    memcpy(&score.kills, &packet[offset], sizeof(score.kills));
    offset += sizeof(score.kills);

    data.scores.push_back(score);
  }

  evt.data = data;
  return evt;
}

Event DecodePLAYER_DISCONNECT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_DISCONNECT;

  PLAYER_DISCONNECT data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  data.reason = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeERROR(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ERROR;

  ERROR data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
  offset += sizeof(data.errorCode);

  uint8_t msgLen = packet[offset++];
  data.message =
      std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_REQUEST(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_REQUEST;

  CHUNK_REQUEST data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.chunkX, &packet[offset], sizeof(data.chunkX));
  offset += sizeof(data.chunkX);

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_DATA(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_DATA;

  CHUNK_DATA data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.chunkX, &packet[offset], sizeof(data.chunkX));
  offset += sizeof(data.chunkX);

  memcpy(&data.chunkWidth, &packet[offset], sizeof(data.chunkWidth));
  offset += sizeof(data.chunkWidth);

  memcpy(&data.chunkHeight, &packet[offset], sizeof(data.chunkHeight));
  offset += sizeof(data.chunkHeight);

  uint32_t tileCount;
  memcpy(&tileCount, &packet[offset], sizeof(tileCount));
  offset += sizeof(tileCount);
  data.tiles.reserve(tileCount);

  for (uint32_t i = 0; i < tileCount; ++i) {
    CHUNK_DATA::Tile tile;
    memcpy(&tile.tileX, &packet[offset], sizeof(tile.tileX));
    offset += sizeof(tile.tileX);

    memcpy(&tile.tileY, &packet[offset], sizeof(tile.tileY));
    offset += sizeof(tile.tileY);

    tile.tileType = packet[offset++];
    tile.tileSprite = packet[offset++];
    tile.tileHealth = packet[offset++];

    data.tiles.push_back(tile);
  }

  evt.data = data;
  return evt;
}

Event DecodePLAYER_INPUT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_INPUT;

  PLAYER_INPUT data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.tick, &packet[offset], sizeof(data.tick));
  offset += sizeof(data.tick);

  data.moveX = packet[offset++];
  data.moveY = packet[offset++];
  data.actions = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeGAME_STATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_STATE;

  GAME_STATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.tick, &packet[offset], sizeof(data.tick));
  offset += sizeof(data.tick);

  uint8_t playerCount = packet[offset++];
  data.players.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    GAME_STATE::PlayerState player;
    memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
    offset += sizeof(player.playerId);

    memcpy(&player.posX, &packet[offset], sizeof(player.posX));
    offset += sizeof(player.posX);

    memcpy(&player.posY, &packet[offset], sizeof(player.posY));
    offset += sizeof(player.posY);

    player.hp = packet[offset++];
    player.shield = packet[offset++];
    player.weapon = packet[offset++];
    player.state = packet[offset++];
    player.sprite = packet[offset++];

    data.players.push_back(player);
  }

  uint8_t enemyCount = packet[offset++];
  data.enemies.reserve(enemyCount);

  for (uint8_t i = 0; i < enemyCount; ++i) {
    GAME_STATE::EnemyState enemy;
    memcpy(&enemy.enemyId, &packet[offset], sizeof(enemy.enemyId));
    offset += sizeof(enemy.enemyId);

    enemy.enemyType = packet[offset++];

    memcpy(&enemy.posX, &packet[offset], sizeof(enemy.posX));
    offset += sizeof(enemy.posX);

    memcpy(&enemy.posY, &packet[offset], sizeof(enemy.posY));
    offset += sizeof(enemy.posY);

    enemy.hp = packet[offset++];
    enemy.state = packet[offset++];
    enemy.pattern = packet[offset++];
    enemy.direction = packet[offset++];
    enemy.flags = packet[offset++];

    data.enemies.push_back(enemy);
  }

  uint8_t projectileCount = packet[offset++];
  data.projectiles.reserve(projectileCount);

  for (uint8_t i = 0; i < projectileCount; ++i) {
    GAME_STATE::ProjectileState projectile;
    memcpy(&projectile.projectileId, &packet[offset],
           sizeof(projectile.projectileId));
    offset += sizeof(projectile.projectileId);

    memcpy(&projectile.ownerId, &packet[offset], sizeof(projectile.ownerId));
    offset += sizeof(projectile.ownerId);

    projectile.type = packet[offset++];

    memcpy(&projectile.posX, &packet[offset], sizeof(projectile.posX));
    offset += sizeof(projectile.posX);

    memcpy(&projectile.posY, &packet[offset], sizeof(projectile.posY));
    offset += sizeof(projectile.posY);

    memcpy(&projectile.velX, &packet[offset], sizeof(projectile.velX));
    offset += sizeof(projectile.velX);

    memcpy(&projectile.velY, &packet[offset], sizeof(projectile.velY));
    offset += sizeof(projectile.velY);

    projectile.damage = packet[offset++];

    data.projectiles.push_back(projectile);
  }

  evt.data = data;
  return evt;
}

Event DecodeENTITY_SPAWN(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ENTITY_SPAWN;

  ENTITY_SPAWN data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.entityId, &packet[offset], sizeof(data.entityId));
  offset += sizeof(data.entityId);

  data.entityType = packet[offset++];
  data.subType = packet[offset++];

  memcpy(&data.posX, &packet[offset], sizeof(data.posX));
  offset += sizeof(data.posX);

  memcpy(&data.posY, &packet[offset], sizeof(data.posY));
  offset += sizeof(data.posY);

  evt.data = data;
  return evt;
}

Event DecodeENTITY_DESTROY(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ENTITY_DESTROY;

  ENTITY_DESTROY data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.entityId, &packet[offset], sizeof(data.entityId));
  offset += sizeof(data.entityId);

  data.destroyType = packet[offset++];

  memcpy(&data.killerId, &packet[offset], sizeof(data.killerId));
  offset += sizeof(data.killerId);

  evt.data = data;
  return evt;
}

Event DecodePLAYER_HIT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_HIT;

  PLAYER_HIT data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  data.damage = packet[offset++];

  memcpy(&data.attackerId, &packet[offset], sizeof(data.attackerId));
  offset += sizeof(data.attackerId);

  data.newHp = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodePOWERUP_COLLECTED(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::POWERUP_COLLECTED;

  POWERUP_COLLECTED data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  memcpy(&data.powerupId, &packet[offset], sizeof(data.powerupId));
  offset += sizeof(data.powerupId);

  data.powerupType = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeFORCE_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::FORCE_UPDATE;

  FORCE_UPDATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);

  data.state = packet[offset++];

  memcpy(&data.posX, &packet[offset], sizeof(data.posX));
  offset += sizeof(data.posX);

  memcpy(&data.posY, &packet[offset], sizeof(data.posY));
  offset += sizeof(data.posY);

  data.level = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeBOSS_SPAWN(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::BOSS_SPAWN;

  BOSS_SPAWN data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.bossId, &packet[offset], sizeof(data.bossId));
  offset += sizeof(data.bossId);

  data.bossType = packet[offset++];

  memcpy(&data.maxHp, &packet[offset], sizeof(data.maxHp));
  offset += sizeof(data.maxHp);

  data.phase = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeBOSS_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::BOSS_UPDATE;

  BOSS_UPDATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.bossId, &packet[offset], sizeof(data.bossId));
  offset += sizeof(data.bossId);

  memcpy(&data.posX, &packet[offset], sizeof(data.posX));
  offset += sizeof(data.posX);

  memcpy(&data.posY, &packet[offset], sizeof(data.posY));
  offset += sizeof(data.posY);

  memcpy(&data.hp, &packet[offset], sizeof(data.hp));
  offset += sizeof(data.hp);

  data.phase = packet[offset++];
  data.action = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeSCROLLING_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::SCROLLING_UPDATE;

  SCROLLING_UPDATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.scrollSpeed, &packet[offset], sizeof(data.scrollSpeed));
  offset += sizeof(data.scrollSpeed);

  memcpy(&data.offsetX, &packet[offset], sizeof(data.offsetX));
  offset += sizeof(data.offsetX);

  evt.data = data;
  return evt;
}

Event DecodeACK(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ACK;

  ACK data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  data.messageType = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_UNLOAD(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_UNLOAD;

  CHUNK_UNLOAD data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.chunkX, &packet[offset], sizeof(data.chunkX));
  offset += sizeof(data.chunkX);

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_TILE_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_TILE_UPDATE;

  CHUNK_TILE_UPDATE data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  memcpy(&data.chunkX, &packet[offset], sizeof(data.chunkX));
  offset += sizeof(data.chunkX);

  memcpy(&data.tileX, &packet[offset], sizeof(data.tileX));
  offset += sizeof(data.tileX);

  memcpy(&data.tileY, &packet[offset], sizeof(data.tileY));
  offset += sizeof(data.tileY);

  data.newTileType = packet[offset++];
  data.newHealth = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_VISIBILITY(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_VISIBILITY;

  CHUNK_VISIBILITY data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  memcpy(&data.sequenceNum, &packet[offset], sizeof(data.sequenceNum));
  offset += sizeof(data.sequenceNum);

  uint8_t chunkCount = packet[offset++];
  data.chunks.reserve(chunkCount);

  for (uint8_t i = 0; i < chunkCount; ++i) {
    int32_t chunkX;
    memcpy(&chunkX, &packet[offset], sizeof(chunkX));
    offset += sizeof(chunkX);
    data.chunks.push_back(chunkX);
  }

  evt.data = data;
  return evt;
}

void SetupDecoder(Decoder& decoder) {
  // TCP Messages
  decoder.registerHandler(0x01, DecodeLOGIN_REQUEST);
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
  decoder.registerHandler(0x03, DecodeSIGNUP_REQUEST);
  decoder.registerHandler(0x04, DecodeSIGNUP_RESPONSE);
  decoder.registerHandler(0x05, DecodeLOGOUT);
  decoder.registerHandler(0x06, DecodeLOBBY_LIST_REQUEST);
  decoder.registerHandler(0x07, DecodeLOBBY_LIST_RESPONSE);
  decoder.registerHandler(0x08, DecodeLOBBY_JOIN);
  decoder.registerHandler(0x09, DecodeLOBBY_UPDATE);
  decoder.registerHandler(0x0A, DecodePLAYER_READY);
  decoder.registerHandler(0x0B, DecodeLOBBY_LEAVE);
  decoder.registerHandler(0x0C, DecodeCHAT_MESSAGE);
  decoder.registerHandler(0x0D, DecodeGAME_LOADING);
  decoder.registerHandler(0x0E, DecodePLAYER_END_LOADING);
  decoder.registerHandler(0x0F, DecodeGAME_START);
  decoder.registerHandler(0x10, DecodeGAME_END);
  decoder.registerHandler(0x11, DecodePLAYER_DISCONNECT);
  decoder.registerHandler(0x12, DecodeERROR);
  decoder.registerHandler(0x13, DecodeCHUNK_REQUEST);
  decoder.registerHandler(0x14, DecodeCHUNK_DATA);

  // UDP Messages
  decoder.registerHandler(0x20, DecodePLAYER_INPUT);
  decoder.registerHandler(0x21, DecodeGAME_STATE);
  decoder.registerHandler(0x22, DecodeENTITY_SPAWN);
  decoder.registerHandler(0x23, DecodeENTITY_DESTROY);
  decoder.registerHandler(0x24, DecodePLAYER_HIT);
  decoder.registerHandler(0x25, DecodePOWERUP_COLLECTED);
  decoder.registerHandler(0x26, DecodeFORCE_UPDATE);
  decoder.registerHandler(0x27, DecodeBOSS_SPAWN);
  decoder.registerHandler(0x28, DecodeBOSS_UPDATE);
  decoder.registerHandler(0x29, DecodeSCROLLING_UPDATE);
  decoder.registerHandler(0x2A, DecodeACK);
  decoder.registerHandler(0x2B, DecodeCHUNK_UNLOAD);
  decoder.registerHandler(0x2C, DecodeCHUNK_TILE_UPDATE);
  decoder.registerHandler(0x2D, DecodeCHUNK_VISIBILITY);
}
