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

  // Serveur → Client
  FORCE_STATE,
  LOGIN_RESPONSE,
  GAME_START,
  GAME_END,
  ERROR,
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

struct ForceState {
  uint16_t forceId;
  uint16_t ownerId;
  float posX;
  float posY;
  uint8_t state;  // 0=AttachedFront, 1=AttachedBack, 2=Detached
};

using ActionData =
    std::variant<std::monostate, AuthUDP, LoginReq, PlayerInput, LoginResponse,
                 GameStart, GameEnd, ErrorMsg, GameState, BossSpawn, BossUpdate,
                 EnemyHit, ForceState>;

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
    case ActionType::GAME_START:
    case ActionType::GAME_END:
    case ActionType::ERROR:
      return 2;  // TCP
    default:
      return 3;
  }
}
