#pragma once
#include <cstdint>
#include <string>
#include <variant>

enum class ActionType : uint8_t {
  AUTH,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  SHOOT,
  FORCE_ATTACH,
  FORCE_DETACH,
  USE_POWERUP,

  LOGIN_REQUEST,
  SIGNUP_REQUEST,
  LOGOUT,
  LOBBY_LIST_REQUEST,
  LOBBY_JOIN,
  PLAYER_READY,
  LOBBY_LEAVE,
  PLAYER_END_LOADING,
  CHUNK_REQUEST
};

struct AuthUDP {
  uint16_t playerId;
};

struct PlayerInput {
  uint32_t tick;
};

struct LoginData {
  std::string username;
  std::string passwordHash;
};

struct SignupData {
  std::string username;
  std::string passwordHash;
  std::string email;
};

struct LogoutData {
  uint16_t playerId;
};

struct LobbyJoinData {
  uint16_t lobbyId;
};

struct PlayerReadyData {
  uint16_t playerId;
  bool ready;
};

struct LobbyLeaveData {
  uint16_t playerId;
};

struct PlayerEndLoadingData {
  uint16_t playerId;
  uint16_t missingChunks;
};

struct ChunkRequestData {
  int32_t chunkX;
};

using ActionData =
    std::variant<std::monostate, AuthUDP, PlayerInput, LoginData, SignupData, LogoutData,
                 LobbyJoinData, PlayerReadyData, LobbyLeaveData,
                 PlayerEndLoadingData, ChunkRequestData>;

struct Action {
  ActionType type;
  ActionData data;
};

inline size_t UseUdp(ActionType type) {
  switch (type) {
    case ActionType::AUTH:
    case ActionType::UP:
    case ActionType::DOWN:
    case ActionType::LEFT:
    case ActionType::RIGHT:
    case ActionType::SHOOT:
    case ActionType::FORCE_ATTACH:
    case ActionType::FORCE_DETACH:
    case ActionType::USE_POWERUP:
      return 0;

    case ActionType::LOGIN_REQUEST:
    case ActionType::SIGNUP_REQUEST:
    case ActionType::LOGOUT:
    case ActionType::LOBBY_LIST_REQUEST:
    case ActionType::LOBBY_JOIN:
    case ActionType::PLAYER_READY:
    case ActionType::LOBBY_LEAVE:
    case ActionType::PLAYER_END_LOADING:
    case ActionType::CHUNK_REQUEST:
      return 2;

    default:
      return 3;
  }
}
