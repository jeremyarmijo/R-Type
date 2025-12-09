#pragma once
#include <arpa/inet.h>
#include <cstring>
#include <vector>

#include "Encode.hpp"

void Auth(const Action& a, std::vector<uint8_t>& out, uint32_t sequenceNum) {
    const auto* auth = std::get_if<AuthUDP>(&a.data);
    if (!auth) return;

    out.resize(4);
    size_t offset = 0;

    uint32_t seq = htonl(sequenceNum);
    memcpy(out.data() + offset, &seq, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint16_t playerId = htons(auth->playerId);
    out.resize(offset + sizeof(uint16_t));
    memcpy(out.data() + offset, &playerId, sizeof(uint16_t));
}

void Player_Up(const Action& a, std::vector<uint8_t>& out,
               uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = -1;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Down(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 1;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Left(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = -1;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Right(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 1;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Shoot(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x01;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void ForceAttach(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x08;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void ForceDetach(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x04;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void UsePowerup(const Action& a, std::vector<uint8_t>& out,
                uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  uint32_t seq = htonl(sequenceNum);
  memcpy(out.data() + offset, &seq, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = htonl(input->tick);
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x02;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void LoginRequest(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* login = std::get_if<LoginData>(&a.data);
  if (!login) return;

  out.clear();
  size_t offset = 0;

  uint8_t usernameLen = static_cast<uint8_t>(login->username.size());
  out.resize(offset + sizeof(uint8_t));
  memcpy(out.data() + offset, &usernameLen, sizeof(uint8_t));
  offset += sizeof(uint8_t);

  out.resize(offset + usernameLen);
  memcpy(out.data() + offset, login->username.data(), usernameLen);
  offset += usernameLen;

  uint8_t passwordLen = static_cast<uint8_t>(login->passwordHash.size());
  out.resize(offset + sizeof(uint8_t));
  memcpy(out.data() + offset, &passwordLen, sizeof(uint8_t));
  offset += sizeof(uint8_t);

  out.resize(offset + passwordLen);
  memcpy(out.data() + offset, login->passwordHash.data(), passwordLen);
}

void SignupRequest(const Action& a, std::vector<uint8_t>& out,
                   uint32_t sequenceNum) {
  const auto* signup = std::get_if<SignupData>(&a.data);
  if (!signup) return;

  out.clear();
  size_t offset = 0;

  uint8_t usernameLen = static_cast<uint8_t>(signup->username.size());
  out.resize(offset + sizeof(uint8_t));
  memcpy(out.data() + offset, &usernameLen, sizeof(uint8_t));
  offset += sizeof(uint8_t);

  out.resize(offset + usernameLen);
  memcpy(out.data() + offset, signup->username.data(), usernameLen);
  offset += usernameLen;

  uint8_t passwordLen = static_cast<uint8_t>(signup->passwordHash.size());
  out.resize(offset + sizeof(uint8_t));
  memcpy(out.data() + offset, &passwordLen, sizeof(uint8_t));
  offset += sizeof(uint8_t);

  out.resize(offset + passwordLen);
  memcpy(out.data() + offset, signup->passwordHash.data(), passwordLen);
  offset += passwordLen;

  uint8_t emailLen = static_cast<uint8_t>(signup->email.size());
  out.resize(offset + sizeof(uint8_t));
  memcpy(out.data() + offset, &emailLen, sizeof(uint8_t));
  offset += sizeof(uint8_t);

  out.resize(offset + emailLen);
  memcpy(out.data() + offset, signup->email.data(), emailLen);
}

void Logout(const Action& a, std::vector<uint8_t>& out, uint32_t sequenceNum) {
  const auto* logout = std::get_if<LogoutData>(&a.data);
  if (!logout) return;

  out.resize(2);
  size_t offset = 0;

  uint16_t playerId = htons(logout->playerId);
  memcpy(out.data() + offset, &playerId, sizeof(uint16_t));
}

void LobbyListRequest(const Action& a, std::vector<uint8_t>& out,
                      uint32_t sequenceNum) {
  out.resize(0);
}

void LobbyJoin(const Action& a, std::vector<uint8_t>& out,
               uint32_t sequenceNum) {
  const auto* join = std::get_if<LobbyJoinData>(&a.data);
  if (!join) return;

  out.resize(2);
  size_t offset = 0;

  uint16_t lobbyId = htons(join->lobbyId);
  memcpy(out.data() + offset, &lobbyId, sizeof(uint16_t));
}

void Ready(const Action& a, std::vector<uint8_t>& out, uint32_t sequenceNum) {
  const auto* ready = std::get_if<PlayerReadyData>(&a.data);
  if (!ready) return;

  out.resize(3);
  size_t offset = 0;

  uint16_t playerId = htons(ready->playerId);
  memcpy(out.data() + offset, &playerId, sizeof(uint16_t));
  offset += sizeof(uint16_t);

  uint8_t readyFlag = ready->ready ? 1 : 0;
  memcpy(out.data() + offset, &readyFlag, sizeof(uint8_t));
}

void LobbyLeave(const Action& a, std::vector<uint8_t>& out,
                uint32_t sequenceNum) {
  const auto* leave = std::get_if<LobbyLeaveData>(&a.data);
  if (!leave) return;

  out.resize(2);
  size_t offset = 0;

  uint16_t playerId = htons(leave->playerId);
  memcpy(out.data() + offset, &playerId, sizeof(uint16_t));
}

void EndLoading(const Action& a, std::vector<uint8_t>& out,
                uint32_t sequenceNum) {
  const auto* endLoading = std::get_if<PlayerEndLoadingData>(&a.data);
  if (!endLoading) return;

  out.resize(4);
  size_t offset = 0;

  uint16_t playerId = htons(endLoading->playerId);
  memcpy(out.data() + offset, &playerId, sizeof(uint16_t));
  offset += sizeof(uint16_t);

  uint16_t missingChunks = htons(endLoading->missingChunks);
  memcpy(out.data() + offset, &missingChunks, sizeof(uint16_t));
}

void ChunkRequest(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* chunk = std::get_if<ChunkRequestData>(&a.data);
  if (!chunk) return;

  out.resize(4);
  size_t offset = 0;

  int32_t chunkX = htonl(chunk->chunkX);
  memcpy(out.data() + offset, &chunkX, sizeof(int32_t));
}

void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::AUTH, Auth);
  encoder.registerHandler(ActionType::UP, Player_Up);
  encoder.registerHandler(ActionType::DOWN, Player_Down);
  encoder.registerHandler(ActionType::LEFT, Player_Left);
  encoder.registerHandler(ActionType::RIGHT, Player_Right);
  encoder.registerHandler(ActionType::SHOOT, Player_Shoot);
  encoder.registerHandler(ActionType::FORCE_ATTACH, ForceAttach);
  encoder.registerHandler(ActionType::FORCE_DETACH, ForceDetach);
  encoder.registerHandler(ActionType::USE_POWERUP, UsePowerup);
  encoder.registerHandler(ActionType::LOGIN_REQUEST, LoginRequest);
  encoder.registerHandler(ActionType::SIGNUP_REQUEST, SignupRequest);
  encoder.registerHandler(ActionType::LOGOUT, Logout);
  encoder.registerHandler(ActionType::LOBBY_LIST_REQUEST, LobbyListRequest);
  encoder.registerHandler(ActionType::LOBBY_JOIN, LobbyJoin);
  encoder.registerHandler(ActionType::PLAYER_READY, Ready);
  encoder.registerHandler(ActionType::LOBBY_LEAVE, LobbyLeave);
  encoder.registerHandler(ActionType::PLAYER_END_LOADING, EndLoading);
  encoder.registerHandler(ActionType::CHUNK_REQUEST, ChunkRequest);
}
