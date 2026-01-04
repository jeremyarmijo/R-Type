#pragma once
#include <arpa/inet.h>

#include <cstring>
#include <vector>

#include "network/Encoder.hpp"

inline void htonf(float value, uint8_t* out) {
  uint32_t asInt;
  static_assert(sizeof(float) == sizeof(uint32_t), "float doit faire 4 bytes");
  std::memcpy(&asInt, &value, sizeof(float));
  asInt = htonl(asInt);
  std::memcpy(out, &asInt, sizeof(uint32_t));
}

inline void Auth(const Action& a, std::vector<uint8_t>& out) {
  const auto* auth = std::get_if<AuthUDP>(&a.data);
  if (!auth) return;

  out.resize(2);
  uint16_t playerId = htons(auth->playerId);
  memcpy(out.data(), &playerId, sizeof(uint16_t));
}

inline void PlayerInputFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(5);
  out[0] = input->up ? 1 : 0;
  out[1] = input->down ? 1 : 0;
  out[2] = input->left ? 1 : 0;
  out[3] = input->right ? 1 : 0;
  out[4] = input->fire ? 0x01 : 0x00;
}

inline void LoginRequestFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* login = std::get_if<LoginReq>(&a.data);
  if (!login) return;

  out.clear();
  size_t offset = 0;

  uint8_t usernameLen = static_cast<uint8_t>(login->username.size());
  out.resize(offset + 1);
  out[offset++] = usernameLen;

  out.resize(offset + usernameLen);
  memcpy(out.data() + offset, login->username.data(), usernameLen);
  offset += usernameLen;

  uint8_t passwordLen = static_cast<uint8_t>(login->passwordHash.size());
  out.resize(offset + 1);
  out[offset++] = passwordLen;

  out.resize(offset + passwordLen);
  memcpy(out.data() + offset, login->passwordHash.data(), passwordLen);
}

inline void GameStateFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* state = std::get_if<GameState>(&a.data);
  if (!state) return;

  out.clear();
  size_t offset = 0;

  // --- Players ---
  uint8_t playerCount = static_cast<uint8_t>(state->players.size());
  out.resize(offset + 1, 0);
  out[offset++] = playerCount;

  for (const auto& player : state->players) {
    out.resize(offset + 15, 0);

    uint16_t playerId = htons(player.playerId);
    memcpy(out.data() + offset, &playerId, sizeof(playerId));
    offset += 2;

    htonf(player.posX, out.data() + offset);
    offset += 4;
    htonf(player.posY, out.data() + offset);
    offset += 4;

    out[offset++] = player.hp;
    out[offset++] = player.shield;
    out[offset++] = player.weapon;
    out[offset++] = player.state;
    out[offset++] = player.sprite;
  }

  // --- Enemies ---
  uint8_t enemyCount = static_cast<uint8_t>(state->enemies.size());
  out.resize(offset + 1, 0);
  out[offset++] = enemyCount;

  for (const auto& enemy : state->enemies) {
    out.resize(offset + 14, 0);

    uint16_t enemyId = htons(enemy.enemyId);
    memcpy(out.data() + offset, &enemyId, sizeof(enemyId));
    offset += 2;

    out[offset++] = enemy.enemyType;

    htonf(enemy.posX, out.data() + offset);
    offset += 4;
    htonf(enemy.posY, out.data() + offset);
    offset += 4;

    out[offset++] = enemy.hp;
    out[offset++] = enemy.state;
    out[offset++] = static_cast<int8_t>(enemy.direction);
  }

  // --- Projectiles ---
  uint8_t projectileCount = static_cast<uint8_t>(state->projectiles.size());
  out.resize(offset + 1, 0);
  out[offset++] = projectileCount;

  for (const auto& proj : state->projectiles) {
    out.resize(offset + 22, 0);

    uint16_t projectileId = htons(proj.projectileId);
    memcpy(out.data() + offset, &projectileId, sizeof(projectileId));
    offset += 2;

    uint16_t ownerId = htons(proj.ownerId);
    memcpy(out.data() + offset, &ownerId, sizeof(ownerId));
    offset += 2;

    out[offset++] = proj.type;

    htonf(proj.posX, out.data() + offset);
    offset += 4;
    htonf(proj.posY, out.data() + offset);
    offset += 4;

    htonf(proj.velX, out.data() + offset);
    offset += 4;
    htonf(proj.velY, out.data() + offset);
    offset += 4;

    out[offset++] = proj.damage;
  }
}

inline void BossSpawnFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* boss = std::get_if<BossSpawn>(&a.data);
  if (!boss) return;

  out.resize(6);
  size_t offset = 0;

  uint16_t bossId = htons(boss->bossId);
  memcpy(out.data() + offset, &bossId, sizeof(uint16_t));
  offset += 2;

  out[offset++] = boss->bossType;

  uint16_t maxHp = htons(boss->maxHp);
  memcpy(out.data() + offset, &maxHp, sizeof(uint16_t));
  offset += 2;

  out[offset++] = boss->phase;
}

inline void BossUpdateFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* boss = std::get_if<BossUpdate>(&a.data);
  if (!boss) return;

  out.resize(14);
  size_t offset = 0;

  uint16_t bossId = htons(boss->bossId);
  memcpy(out.data() + offset, &bossId, sizeof(uint16_t));
  offset += 2;

  htonf(boss->posX, out.data() + offset);
  offset += 4;
  htonf(boss->posY, out.data() + offset);
  offset += 4;

  uint16_t hp = htons(boss->hp);
  memcpy(out.data() + offset, &hp, sizeof(uint16_t));
  offset += 2;

  out[offset++] = boss->phase;
  out[offset++] = boss->action;
}

inline void EnemyHitFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* hit = std::get_if<EnemyHit>(&a.data);
  if (!hit) return;

  out.resize(5);
  size_t offset = 0;

  uint16_t enemyId = htons(hit->enemyId);
  memcpy(out.data() + offset, &enemyId, sizeof(uint16_t));
  offset += 2;

  out[offset++] = hit->damage;

  uint16_t hp = htons(hit->hpRemaining);
  memcpy(out.data() + offset, &hp, sizeof(uint16_t));
}

inline void GameStartFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* start = std::get_if<GameStart>(&a.data);
  if (!start) return;

  out.resize(12);
  size_t offset = 0;

  htonf(start->playerSpawnX, out.data() + offset);
  offset += 4;

  htonf(start->playerSpawnY, out.data() + offset);
  offset += 4;

  htonf(start->scrollSpeed, out.data() + offset);
}

inline void GameEndFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* end = std::get_if<GameEnd>(&a.data);
  if (!end) return;

  uint8_t playerCount = end->scores.size();

  out.resize(2 + playerCount * 7);
  size_t offset = 0;
  out[offset++] = end->victory ? 1 : 0;
  out[offset++] = playerCount;

  for (const auto& s : end->scores) {
    uint16_t playerId = htons(s.playerId);
    memcpy(out.data() + offset, &playerId, 2);
    offset += 2;

    uint32_t score = htonl(s.score);
    memcpy(out.data() + offset, &score, 4);
    offset += 4;

    out[offset++] = s.rank;
  }
}

inline void ErrorFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* err = std::get_if<ErrorMsg>(&a.data);
  if (!err) return;

  out.clear();
  size_t offset = 0;

  out.resize(offset + 2);
  uint16_t code = htons(err->errorCode);
  memcpy(out.data() + offset, &code, sizeof(uint16_t));
  offset += 2;

  uint8_t msgLen = static_cast<uint8_t>(err->message.size());
  out.resize(offset + 1);
  out[offset++] = msgLen;

  out.resize(offset + msgLen);
  memcpy(out.data() + offset, err->message.data(), msgLen);
}

inline void LoginResponseFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* resp = std::get_if<LoginResponse>(&a.data);
  if (!resp) return;

  out.clear();
  size_t offset = 0;

  out.resize(offset + 1);
  out[offset++] = resp->success ? 1 : 0;

  // SUCCESS
  if (resp->success) {
    out.resize(offset + 4);

    uint16_t playerId = htons(resp->playerId);
    memcpy(out.data() + offset, &playerId, 2);
    offset += 2;

    uint16_t udpPort = htons(resp->udpPort);
    memcpy(out.data() + offset, &udpPort, 2);
    return;
  }

  // FAILURE
  out.resize(offset + 2);
  uint16_t code = htons(resp->errorCode);
  memcpy(out.data() + offset, &code, 2);
  offset += 2;

  uint8_t msgLen = static_cast<uint8_t>(resp->message.size());
  out.resize(offset + 1);
  out[offset++] = msgLen;

  out.resize(offset + msgLen);
  memcpy(out.data() + offset, resp->message.data(), msgLen);
}

inline void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::AUTH, Auth);
  encoder.registerHandler(ActionType::ERROR, ErrorFunc);
  encoder.registerHandler(ActionType::GAME_START, GameStartFunc);
  encoder.registerHandler(ActionType::GAME_END, GameEndFunc);
  encoder.registerHandler(ActionType::UP_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::UP_RELEASE, PlayerInputFunc);
  encoder.registerHandler(ActionType::DOWN_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::DOWN_RELEASE, PlayerInputFunc);
  encoder.registerHandler(ActionType::LEFT_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::LEFT_RELEASE, PlayerInputFunc);
  encoder.registerHandler(ActionType::RIGHT_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::RIGHT_RELEASE, PlayerInputFunc);
  encoder.registerHandler(ActionType::FIRE_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::FIRE_RELEASE, PlayerInputFunc);
  encoder.registerHandler(ActionType::LOGIN_REQUEST, LoginRequestFunc);
  encoder.registerHandler(ActionType::LOGIN_RESPONSE, LoginResponseFunc);
  encoder.registerHandler(ActionType::GAME_STATE, GameStateFunc);
  encoder.registerHandler(ActionType::BOSS_SPAWN, BossSpawnFunc);
  encoder.registerHandler(ActionType::BOSS_UPDATE, BossUpdateFunc);
  encoder.registerHandler(ActionType::ENEMY_HIT, EnemyHitFunc);
}
