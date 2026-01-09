#pragma once
#include <arpa/inet.h>

#include <cstring>
#include <vector>

#include "network/Encoder.hpp"

void htonf(float value, uint8_t* out);

void Auth(const Action& a, std::vector<uint8_t>& out);
void PlayerInputFunc(const Action& a, std::vector<uint8_t>& out);
void LoginRequestFunc(const Action& a, std::vector<uint8_t>& out);
void GameStateFunc(const Action& a, std::vector<uint8_t>& out);
void BossSpawnFunc(const Action& a, std::vector<uint8_t>& out);
void BossUpdateFunc(const Action& a, std::vector<uint8_t>& out);
void EnemyHitFunc(const Action& a, std::vector<uint8_t>& out);
void GameStartFunc(const Action& a, std::vector<uint8_t>& out);
void GameEndFunc(const Action& a, std::vector<uint8_t>& out);
void ErrorFunc(const Action& a, std::vector<uint8_t>& out);
void LoginResponseFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyCreateFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyJoinRequestFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyJoinResponseFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyListRequestFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyListResponseFunc(const Action& a, std::vector<uint8_t>& out);
void PlayerReadyFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyUpdateFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyLeaveFunc(const Action& a, std::vector<uint8_t>& out);
void LobbyStartFunc(const Action& a, std::vector<uint8_t>& out);

void SetupEncoder(Encoder& encoder);
