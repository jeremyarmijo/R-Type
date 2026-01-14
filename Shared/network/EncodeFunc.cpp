#include "network/EncodeFunc.hpp"

#include <vector>

void htonf(float value, uint8_t* out) {
  uint32_t asInt;
  static_assert(sizeof(float) == sizeof(uint32_t), "float doit faire 4 bytes");
  std::memcpy(&asInt, &value, sizeof(float));
  asInt = htonl(asInt);
  std::memcpy(out, &asInt, sizeof(uint32_t));
}

void Auth(const Action& a, std::vector<uint8_t>& out) {
  const auto* auth = std::get_if<AuthUDP>(&a.data);
  if (!auth) return;
  out.resize(2);
  uint16_t playerId = htons(auth->playerId);
  memcpy(out.data(), &playerId, sizeof(uint16_t));
}

void PlayerInputFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;
  out.resize(5);
  out[0] = input->up ? 1 : 0;
  out[1] = input->down ? 1 : 0;
  out[2] = input->left ? 1 : 0;
  out[3] = input->right ? 1 : 0;
  out[4] = input->fire ? 0x01 : 0x00;
}

void LoginRequestFunc(const Action& a, std::vector<uint8_t>& out) {
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

void GameStateFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* state = std::get_if<GameState>(&a.data);
  if (!state) return;
  out.clear();
  size_t offset = 0;
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

void BossSpawnFunc(const Action& a, std::vector<uint8_t>& out) {
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

void BossUpdateFunc(const Action& a, std::vector<uint8_t>& out) {
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

void EnemyHitFunc(const Action& a, std::vector<uint8_t>& out) {
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

void GameStartFunc(const Action& a, std::vector<uint8_t>& out) {
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

void GameEndFunc(const Action& a, std::vector<uint8_t>& out) {
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

void ErrorFunc(const Action& a, std::vector<uint8_t>& out) {
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

void LoginResponseFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* resp = std::get_if<LoginResponse>(&a.data);
  if (!resp) return;
  out.clear();
  size_t offset = 0;
  out.resize(offset + 1);
  out[offset++] = resp->success ? 1 : 0;
  if (resp->success) {
    out.resize(offset + 4);
    uint16_t playerId = htons(resp->playerId);
    memcpy(out.data() + offset, &playerId, 2);
    offset += 2;
    uint16_t udpPort = htons(resp->udpPort);
    memcpy(out.data() + offset, &udpPort, 2);
    return;
  }
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

void LobbyCreateFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* create = std::get_if<LobbyCreate>(&a.data);
  if (!create) return;

  out.clear();

  uint8_t lobbyNameLen = static_cast<uint8_t>(create->lobbyName.size());
  out.push_back(lobbyNameLen);
  out.insert(out.end(), create->lobbyName.begin(), create->lobbyName.end());

  uint8_t playerNameLen = static_cast<uint8_t>(create->lobbyPlayer.size());
  out.push_back(playerNameLen);
  out.insert(out.end(), create->lobbyPlayer.begin(), create->lobbyPlayer.end());

  uint8_t passwordLen = static_cast<uint8_t>(create->password.size());
  out.push_back(passwordLen);
  out.insert(out.end(), create->password.begin(), create->password.end());

  out.push_back(create->Maxplayer);

  out.push_back(create->difficulty);
}

void LobbyJoinRequestFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* req = std::get_if<LobbyJoinRequest>(&a.data);
  if (!req) return;

  out.clear();

  uint16_t lobbyIdNet = htons(req->lobbyId);
  uint8_t idBytes[2];
  memcpy(idBytes, &lobbyIdNet, 2);
  out.push_back(idBytes[0]);
  out.push_back(idBytes[1]);

  uint8_t nameLen = static_cast<uint8_t>(req->name.size());
  out.push_back(nameLen);
  out.insert(out.end(), req->name.begin(), req->name.end());

  uint8_t passwordLen = static_cast<uint8_t>(req->password.size());
  out.push_back(passwordLen);
  out.insert(out.end(), req->password.begin(), req->password.end());
}

void LobbyJoinResponseFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* resp = std::get_if<LobbyJoinResponse>(&a.data);
  if (!resp) return;
  out.clear();
  size_t offset = 0;
  out.resize(offset + 1);
  out[offset++] = resp->success ? 1 : 0;
  if (resp->success) {
    out.resize(offset + 4);
    uint16_t lobbyId = htons(resp->lobbyId);
    memcpy(out.data() + offset, &lobbyId, 2);
    offset += 2;
    uint16_t playerId = htons(resp->playerId);
    memcpy(out.data() + offset, &playerId, 2);
    offset += 2;
    uint8_t playerCount = static_cast<uint8_t>(resp->players.size());
    out.resize(offset + 1);
    out[offset++] = playerCount;
    for (const auto& player : resp->players) {
      out.resize(offset + 3);
      uint16_t pId = htons(player.playerId);
      memcpy(out.data() + offset, &pId, 2);
      offset += 2;
      out[offset++] = player.ready ? 1 : 0;
      uint8_t usernameLen = static_cast<uint8_t>(player.username.size());
      out.resize(offset + 1);
      out[offset++] = usernameLen;
      out.resize(offset + usernameLen);
      memcpy(out.data() + offset, player.username.data(), usernameLen);
      offset += usernameLen;
    }
  } else {
    out.resize(offset + 2);
    uint16_t code = htons(resp->errorCode);
    memcpy(out.data() + offset, &code, 2);
    offset += 2;
    uint8_t msgLen = static_cast<uint8_t>(resp->errorMessage.size());
    out.resize(offset + 1);
    out[offset++] = msgLen;
    out.resize(offset + msgLen);
    memcpy(out.data() + offset, resp->errorMessage.data(), msgLen);
  }
}

void LobbyListResponseFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* resp = std::get_if<LobbyListResponse>(&a.data);
  if (!resp) return;

  out.clear();
  size_t offset = 0;

  uint8_t lobbyCount = static_cast<uint8_t>(resp->lobbies.size());
  out.push_back(lobbyCount);
  offset++;

  for (const auto& lobby : resp->lobbies) {
    uint16_t lobbyId = htons(lobby.lobbyId);
    size_t startIdx = out.size();
    out.resize(startIdx + 2);
    memcpy(out.data() + startIdx, &lobbyId, 2);

    uint8_t nameLen = static_cast<uint8_t>(lobby.name.size());
    out.push_back(nameLen);
    out.insert(out.end(), lobby.name.begin(), lobby.name.end());

    out.push_back(lobby.playerCount);
    out.push_back(lobby.maxPlayers);
    out.push_back(lobby.difficulty);
    out.push_back(lobby.isStarted ? 1 : 0);
    out.push_back(lobby.hasPassword ? 1 : 0);
  }
}

void PlayerReadyFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* ready = std::get_if<PlayerReady>(&a.data);
  if (!ready) return;
  out.resize(1);
  out[0] = ready->ready ? 1 : 0;
}

void LobbyUpdateFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* update = std::get_if<LobbyUpdate>(&a.data);
  if (!update) return;

  out.clear();
  size_t offset = 0;

  uint8_t nameLen = static_cast<uint8_t>(update->name.size());
  out.push_back(nameLen);
  out.insert(out.end(), update->name.begin(), update->name.end());
  offset = out.size();

  uint16_t hId = htons(update->hostId);
  out.resize(offset + sizeof(uint16_t));
  memcpy(out.data() + offset, &hId, sizeof(uint16_t));
  offset += sizeof(uint16_t);

  out.push_back(update->asStarted ? 1 : 0);
  out.push_back(update->maxPlayers);
  out.push_back(update->difficulty);

  uint8_t playerCount = static_cast<uint8_t>(update->playerInfo.size());
  out.push_back(playerCount);
  offset = out.size();

  for (const auto& player : update->playerInfo) {
    uint16_t pId = htons(player.playerId);
    size_t currentSize = out.size();
    out.resize(currentSize + 2);
    memcpy(out.data() + currentSize, &pId, sizeof(uint16_t));

    out.push_back(player.ready ? 1 : 0);

    uint8_t uLen = static_cast<uint8_t>(player.username.size());
    out.push_back(uLen);
    out.insert(out.end(), player.username.begin(), player.username.end());
  }
}

void LobbyStartFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* start = std::get_if<LobbyStart>(&a.data);
  if (!start) return;
  out.resize(1);
  out[0] = start->countdown;
}

void LobbyListRequestFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* req = std::get_if<LobbyListRequest>(&a.data);
  if (!req) return;

  out.clear();
  uint16_t id = htons(req->playerId);
  uint8_t buf[2];
  memcpy(buf, &id, 2);
  out.push_back(buf[0]);
  out.push_back(buf[1]);
}

void LobbyLeaveFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* req = std::get_if<LobbyLeave>(&a.data);
  if (!req) return;

  out.clear();
  uint16_t id = htons(req->playerId);
  uint8_t buf[2];
  memcpy(buf, &id, 2);
  out.push_back(buf[0]);
  out.push_back(buf[1]);
}

void LobbyKickFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* kick = std::get_if<LobbyKick>(&a.data);
  if (!kick) return;

  out.clear();

  uint16_t pId = htons(kick->playerId);

  out.resize(sizeof(uint16_t));
  memcpy(out.data(), &pId, sizeof(uint16_t));
}

void MessageFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* msg = std::get_if<Message>(&a.data);
  if (!msg) return;

  out.clear();
  size_t offset = 0;

  out.resize(offset + 2);
  uint16_t lobbyId = htons(msg->lobbyId);
  memcpy(out.data() + offset, &lobbyId, sizeof(lobbyId));
  offset += 2;

  uint8_t nameLen = static_cast<uint8_t>(msg->playerName.size());
  out.resize(offset + 1);
  out[offset++] = nameLen;
  out.resize(offset + nameLen);
  memcpy(out.data() + offset, msg->playerName.data(), nameLen);
  offset += nameLen;

  uint8_t msgContentLen = static_cast<uint8_t>(msg->message.size());
  out.resize(offset + 1);
  out[offset++] = msgContentLen;
  out.resize(offset + msgContentLen);
  memcpy(out.data() + offset, msg->message.data(), msgContentLen);
}

void MapDataFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* map = std::get_if<MapData>(&a.data);
  if (!map) return;

  out.clear();

  uint16_t w = htons(map->width);
  uint8_t wBytes[2];
  memcpy(wBytes, &w, 2);
  out.insert(out.end(), wBytes, wBytes + 2);

  uint16_t h = htons(map->height);
  uint8_t hBytes[2];
  memcpy(hBytes, &h, 2);
  out.insert(out.end(), hBytes, hBytes + 2);

  uint8_t speedBytes[4];
  htonf(map->scrollSpeed, speedBytes);
  out.insert(out.end(), speedBytes, speedBytes + 4);

  if (!map->tiles.empty()) {
    out.insert(out.end(), map->tiles.begin(), map->tiles.end());
  }
}

void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::AUTH, Auth);
  encoder.registerHandler(ActionType::LOBBY_LEAVE, LobbyLeaveFunc);
  encoder.registerHandler(ActionType::LOBBY_LIST_REQUEST, LobbyListRequestFunc);
  encoder.registerHandler(ActionType::ERROR_SERVER, ErrorFunc);
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
  encoder.registerHandler(ActionType::LOBBY_CREATE, LobbyCreateFunc);
  encoder.registerHandler(ActionType::LOBBY_KICK, LobbyKickFunc);
  encoder.registerHandler(ActionType::LOBBY_JOIN_REQUEST, LobbyJoinRequestFunc);
  encoder.registerHandler(ActionType::LOBBY_JOIN_RESPONSE,
                          LobbyJoinResponseFunc);
  encoder.registerHandler(ActionType::SEND_MAP, MapDataFunc);
  encoder.registerHandler(ActionType::LOBBY_LIST_REQUEST, LobbyListRequestFunc);
  encoder.registerHandler(ActionType::LOBBY_LIST_RESPONSE,
                          LobbyListResponseFunc);
  encoder.registerHandler(ActionType::MESSAGE, MessageFunc);
  encoder.registerHandler(ActionType::PLAYER_READY, PlayerReadyFunc);
  encoder.registerHandler(ActionType::LOBBY_UPDATE, LobbyUpdateFunc);
  encoder.registerHandler(ActionType::LOBBY_LEAVE, LobbyLeaveFunc);
  encoder.registerHandler(ActionType::LOBBY_START, LobbyStartFunc);
}
