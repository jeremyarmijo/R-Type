#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

// -------------------------
// TCP Messages
// -------------------------

struct LOGIN_REQUEST {
  std::string username;
  std::string password;
};

struct LOGIN_RESPONSE {
  uint8_t success;
  uint16_t playerId;
  uint16_t udpPort;
  uint16_t errorCode;
  std::string message;
};

struct GAME_START {
  float playerSpawnX;
  float playerSpawnY;
  float scrollSpeed;
};

struct GAME_END {
  struct Score {
    uint16_t playerId;
    uint32_t score;
    uint8_t rank;
  };
  uint8_t victory;
  std::vector<Score> scores;
};

struct ERROR {
  uint16_t errorCode;
  std::string message;
};

// -------------------------
// UDP Messages
// -------------------------

struct PLAYER_INPUT {
  uint8_t up;
  uint8_t down;
  uint8_t left;
  uint8_t right;
  uint8_t fire;
};

struct GAME_STATE {
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
  std::vector<PlayerState> players;
  std::vector<EnemyState> enemies;
  std::vector<ProjectileState> projectiles;
};

struct AUTH {
  uint16_t playerId;
};

struct BOSS_SPAWN {
  uint16_t bossId;
  uint8_t bossType;
  uint16_t maxHp;
  uint8_t phase;
};

struct BOSS_UPDATE {
  uint16_t bossId;
  float posX;
  float posY;
  uint16_t hp;
  uint8_t phase;
  uint8_t action;
};

struct ENEMY_HIT {
  uint16_t enemyId;
  uint8_t damage;
  uint16_t hpRemaining;
};

enum class EventType : uint8_t {
  // TCP Messages
  LOGIN_REQUEST = 0x01,
  LOGIN_RESPONSE = 0x02,
  GAME_START = 0x0F,
  GAME_END = 0x10,
  ERROR = 0x12,

  // UDP Messages
  PLAYER_INPUT = 0x20,
  GAME_STATE = 0x21,
  AUTH = 0x22,
  BOSS_SPAWN = 0x23,
  BOSS_UPDATE = 0x24,
  ENEMY_HIT = 0x25,

  UNKNOWN = 0xFF
};

using EventData =
    std::variant<std::monostate, LOGIN_REQUEST, LOGIN_RESPONSE, GAME_START,
                 GAME_END, ERROR, PLAYER_INPUT, GAME_STATE, AUTH, BOSS_SPAWN,
                 BOSS_UPDATE, ENEMY_HIT>;

struct Event {
  EventType type;
  EventData data;
};
