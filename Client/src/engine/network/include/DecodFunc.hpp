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
  payloadLength = ntohl(payloadLength);
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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  evt.type = EventType::ERROR;

  ERROR data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

  data.up = packet[offset++];
  data.down = packet[offset++];
  data.left = packet[offset++];
  data.right = packet[offset++];
  data.fire = packet[offset++];

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

  uint8_t playerCount = packet[offset++];
  data.players.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    GAME_STATE::PlayerState player;
    memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
    player.playerId = ntohs(player.playerId);
    offset += sizeof(player.playerId);

    uint32_t temp;
    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&player.posX, &temp, sizeof(player.posX));
    offset += sizeof(temp);

    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&player.posY, &temp, sizeof(player.posY));
    offset += sizeof(temp);

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
    enemy.enemyId = ntohs(enemy.enemyId);
    offset += sizeof(enemy.enemyId);

    enemy.enemyType = packet[offset++];

    uint32_t temp;
    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&enemy.posX, &temp, sizeof(enemy.posX));
    offset += sizeof(temp);

    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&enemy.posY, &temp, sizeof(enemy.posY));
    offset += sizeof(temp);

    enemy.hp = packet[offset++];
    enemy.state = packet[offset++];
    enemy.direction = packet[offset++];

    data.enemies.push_back(enemy);
  }

  uint8_t projectileCount = packet[offset++];
  data.projectiles.reserve(projectileCount);

  for (uint8_t i = 0; i < projectileCount; ++i) {
    GAME_STATE::ProjectileState projectile;
    memcpy(&projectile.projectileId, &packet[offset],
           sizeof(projectile.projectileId));
    projectile.projectileId = ntohs(projectile.projectileId);
    offset += sizeof(projectile.projectileId);

    memcpy(&projectile.ownerId, &packet[offset], sizeof(projectile.ownerId));
    projectile.ownerId = ntohs(projectile.ownerId);
    offset += sizeof(projectile.ownerId);

    projectile.type = packet[offset++];

    uint32_t temp;
    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&projectile.posX, &temp, sizeof(projectile.posX));
    offset += sizeof(temp);

    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&projectile.posY, &temp, sizeof(projectile.posY));
    offset += sizeof(temp);

    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&projectile.velX, &temp, sizeof(projectile.velX));
    offset += sizeof(temp);

    memcpy(&temp, &packet[offset], sizeof(temp));
    temp = ntohl(temp);
    memcpy(&projectile.velY, &temp, sizeof(projectile.velY));
    offset += sizeof(temp);

    projectile.damage = packet[offset++];

    data.projectiles.push_back(projectile);
  }

  evt.data = data;
  return evt;
}

Event DecodeAUTH(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::AUTH;

  AUTH data;
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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
  size_t offset = 2;

  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) {
    return Event{};
  }

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

void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x01, DecodeLOGIN_REQUEST);
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
  decoder.registerHandler(0x0F, DecodeGAME_START);
  decoder.registerHandler(0x10, DecodeGAME_END);
  decoder.registerHandler(0x12, DecodeERROR);

  decoder.registerHandler(0x20, DecodePLAYER_INPUT);
  decoder.registerHandler(0x21, DecodeGAME_STATE);
  decoder.registerHandler(0x22, DecodeAUTH);
  decoder.registerHandler(0x23, DecodeBOSS_SPAWN);
  decoder.registerHandler(0x24, DecodeBOSS_UPDATE);
  decoder.registerHandler(0x25, DecodeENEMY_HIT);
}
