#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

enum class ActionType : uint8_t {
  // Client → Serveur
  AUTH,
  UP_PRESS,
  UP_RELEASE,
  DOWN_PRESS,
  DOWN_RELEASE,
  LEFT_PRESS,
  LEFT_RELEASE,
  RIGHT_PRESS,
  RIGHT_RELEASE,
  FIRE_PRESS,
  FIRE_RELEASE,
  LOGIN_REQUEST,
  LOBBY_CREATE,
  LOBBY_JOIN_REQUEST,
  LOBBY_LIST_REQUEST,
  PLAYER_READY,
  LOBBY_LEAVE,
  MESSAGE,
  LOBBY_KICK,

  // Serveur → Client
  FORCE_STATE,
  LOGIN_RESPONSE,
  LOBBY_JOIN_RESPONSE,
  LOBBY_LIST_RESPONSE,
  LOBBY_UPDATE,
  LOBBY_START,
  GAME_START,
  GAME_END,
  ERROR_SERVER,
  GAME_STATE,
  BOSS_SPAWN,
  BOSS_UPDATE,
  ENEMY_HIT
};

struct AuthUDP {
  uint16_t playerId;
};

struct LoginReq {
  std::string username;
  std::string passwordHash;
};

struct PlayerInput {
  bool up;
  bool down;
  bool left;
  bool right;
  uint8_t fire;

  PlayerInput() : up(false), down(false), left(false), right(false), fire(0) {}
};

struct LoginResponse {
  bool success;
  uint16_t playerId;
  uint16_t udpPort;
  uint16_t errorCode;
  std::string message;
};

struct LobbyCreate {
  std::string lobbyName;
  std::string lobbyPlayer;
  std::string password;
  uint8_t Maxplayer;
  uint8_t difficulty;
};

struct LobbyJoinRequest {
  uint16_t lobbyId;
  std::string name;
  std::string password;
};

struct LobbyPlayer {
  uint16_t playerId;
  bool ready;
  std::string username;
};

struct LobbyJoinResponse {
  bool success;
  uint16_t lobbyId;
  uint16_t playerId;
  std::vector<LobbyPlayer> players;
  uint16_t errorCode;
  std::string errorMessage;
};

struct LobbyInfo {
  uint16_t lobbyId;
  std::string name;
  uint8_t playerCount;
  uint8_t maxPlayers;
  uint8_t difficulty;
  bool isStarted;
  bool hasPassword;
};

struct Message {
  uint16_t lobbyId;
  std::string playerName;
  std::string message;
};

struct LobbyListRequest {
  uint16_t playerId;
};

struct LobbyLeave {
  uint16_t playerId;
};

struct LobbyListResponse {
  std::vector<LobbyInfo> lobbies;
};

struct PlayerReady {
  bool ready;
};

struct LobbyUpdate {
  std::string name;
  uint16_t hostId;
  bool asStarted;
  uint8_t maxPlayers;
  uint8_t difficulty;
  std::vector<LobbyPlayer> playerInfo;
};

struct LobbyStart {
  uint8_t countdown;
};

struct GameStart {
  float playerSpawnX;
  float playerSpawnY;
  float scrollSpeed;
};

struct GameEndScore {
  uint16_t playerId;
  uint32_t score;
  uint8_t rank;
};

struct GameEnd {
  bool victory;
  std::vector<GameEndScore> scores;
};

struct ErrorMsg {
  uint16_t errorCode;
  std::string message;
};

struct PlayerState {
  uint16_t playerId;
  float posX;
  float posY;
  uint8_t hp;
  uint8_t shield;
  uint8_t weapon;
  uint8_t state;
  uint8_t sprite;
};

struct EnemyState {
  uint16_t enemyId;
  uint8_t enemyType;
  float posX;
  float posY;
  uint8_t hp;
  uint8_t state;
  int8_t direction;
};

struct ProjectileState {
  uint16_t projectileId;
  uint16_t ownerId;
  uint8_t type;
  float posX;
  float posY;
  float velX;
  float velY;
  uint8_t damage;
};

struct GameState {
  std::vector<PlayerState> players;
  std::vector<EnemyState> enemies;
  std::vector<ProjectileState> projectiles;
};

struct BossSpawn {
  uint16_t bossId;
  uint8_t bossType;
  uint16_t maxHp;
  uint8_t phase;
};

struct BossUpdate {
  uint16_t bossId;
  float posX;
  float posY;
  uint16_t hp;
  uint8_t phase;
  uint8_t action;
};

struct EnemyHit {
  uint16_t enemyId;
  uint8_t damage;
  uint16_t hpRemaining;
};

struct LobbyKick {
  uint16_t playerId;
};

struct ForceState {
  uint16_t forceId;
  uint16_t ownerId;
  float posX;
  float posY;
  uint8_t state;  // 0=AttachedFront, 1=AttachedBack, 2=Detached
};
using ActionData =
    std::variant<std::monostate, AuthUDP, LoginReq, PlayerInput, LoginResponse,
                 LobbyCreate, LobbyJoinRequest, LobbyJoinResponse,
                 LobbyListResponse, PlayerReady, LobbyUpdate, LobbyStart,
                 GameStart, GameEnd, ErrorMsg, GameState, BossSpawn, BossUpdate,
                 EnemyHit, LobbyListRequest, LobbyLeave, Message, LobbyKick,
                 ForceState>;

struct Action {
  ActionType type;
  ActionData data;
};

inline size_t UseUdp(ActionType type) {
  switch (type) {
    case ActionType::AUTH:
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
    case ActionType::GAME_STATE:
    case ActionType::BOSS_SPAWN:
    case ActionType::BOSS_UPDATE:
    case ActionType::ENEMY_HIT:
    case ActionType::FORCE_STATE:
      return 0;  // UDP

    case ActionType::LOGIN_REQUEST:
    case ActionType::LOGIN_RESPONSE:
    case ActionType::LOBBY_CREATE:
    case ActionType::LOBBY_JOIN_REQUEST:
    case ActionType::LOBBY_JOIN_RESPONSE:
    case ActionType::LOBBY_LIST_REQUEST:
    case ActionType::LOBBY_LIST_RESPONSE:
    case ActionType::PLAYER_READY:
    case ActionType::LOBBY_UPDATE:
    case ActionType::LOBBY_LEAVE:
    case ActionType::LOBBY_START:
    case ActionType::GAME_START:
    case ActionType::GAME_END:
    case ActionType::MESSAGE:
    case ActionType::LOBBY_KICK:
    case ActionType::ERROR_SERVER:
      return 2;  // TCP

    default:
      return 3;
  }
}
