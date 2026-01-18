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

struct ERROR_EVNT {
  uint16_t errorCode;
  std::string message;
};

// -------------------------
// UDP Messages
// -------------------------

struct PLAYER_INPUT {
  bool up;
  bool down;
  bool left;
  bool right;
  uint8_t fire;
};

struct GAME_STATE {
  struct PlayerState {
    uint16_t playerId;
    uint16_t mask;
    float posX;
    float posY;
    uint8_t hp;
    uint8_t shield;
    uint8_t weapon;
    uint8_t state;
    uint8_t sprite;
    uint32_t score = 0;
  };
  struct EnemyState {
    uint16_t enemyId;
    uint16_t mask;
    uint8_t enemyType;
    float posX;
    float posY;
    uint8_t hp;
    uint8_t state;
    int8_t direction;
  };
  struct ProjectileState {
    uint16_t projectileId;
    uint16_t mask;
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

struct MESSAGE {
  uint16_t lobbyId;
  std::string playerName;
  std::string message;
};

struct LOBBY_CREATE {
  std::string lobbyName;
  std::string playerName;
  std::string password;
  uint8_t Maxplayer;
  uint8_t difficulty;
};

struct LOBBY_JOIN_REQUEST {
  uint16_t lobbyId;
  std::string name;
  std::string password;
};

struct LOBBY_JOIN_RESPONSE {
  struct Player {
    uint16_t playerId;
    bool ready;
    std::string username;
  };

  uint8_t success;
  uint16_t lobbyId;
  uint16_t playerId;
  std::vector<Player> players;

  uint16_t errorCode;
  std::string errorMessage;
};

struct Lobbies {
  uint16_t lobbyId;
  std::string name;
  uint8_t playerCount;
  uint8_t maxPlayers;
  uint8_t difficulty;
  bool isStarted;
  bool hasPassword;
};

struct LOBBY_LIST_RESPONSE {
  std::vector<Lobbies> lobbies;
};

struct PLAYER_READY {
  bool ready;
};

struct PlayerInfo {
  uint16_t playerId;
  bool ready;
  std::string username;
};

struct LOBBY_UPDATE {
  std::string name;
  uint16_t hostId;
  bool asStarted;
  uint8_t maxPlayers;
  uint8_t difficulty;
  std::vector<PlayerInfo> playerInfo;
};

struct LOBBY_START {
  uint8_t countdown;
};

struct LOBBY_LIST_REQUEST {
  uint16_t playerId;
};

struct LOBBY_LEAVE {
  uint16_t playerId;
};

struct LOBBY_KICK {
  uint16_t playerId;
};

struct CLIENT_LEAVE {
  uint16_t playerId;
};

struct MAP_DATA {
  uint16_t width;
  uint16_t height;
  float scrollSpeed;
  std::vector<uint8_t> tiles;
};

struct LEVEL_TRANSITION {
  uint8_t levelNumber;
};

enum class EventType : uint8_t {
  // TCP Messages
  LOGIN_REQUEST = 0x01,
  LOGIN_RESPONSE = 0x02,

  LOBBY_CREATE = 0x03,
  LOBBY_JOIN_REQUEST = 0x04,
  LOBBY_JOIN_RESPONSE = 0x05,
  LOBBY_LIST_REQUEST = 0x06,
  LOBBY_LIST_RESPONSE = 0x07,
  PLAYER_READY = 0x08,
  LOBBY_UPDATE = 0x09,
  LOBBY_LEAVE = 0x0A,
  LOBBY_START = 0x0B,
  MESSAGE = 0x0C,
  LOBBY_KICK = 0x0D,
  SEND_MAP = 0x0E,
  GAME_START = 0x0F,
  GAME_END = 0x10,
  CLIENT_LEAVE = 0x11,
  ERROR_TYPE = 0x12,

  // UDP Messages
  PLAYER_INPUT = 0x20,
  GAME_STATE = 0x21,
  AUTH = 0x22,
  BOSS_SPAWN = 0x23,
  BOSS_UPDATE = 0x24,
  ENEMY_HIT = 0x25,
  FORCE_STATE = 0x26,
  LEVEL_TRANSITION = 0x027,

  UNKNOWN = 0xFF
};

struct FORCE_STATE {
  uint16_t forceId;
  uint16_t ownerId;
  float posX;
  float posY;
  uint8_t state;  // 0=AttachedFront, 1=AttachedBack, 2=Detached
};

using EventData = std::variant<
    std::monostate, LOGIN_REQUEST, LOGIN_RESPONSE, GAME_START, GAME_END,
    ERROR_EVNT, PLAYER_INPUT, GAME_STATE, AUTH, BOSS_UPDATE, ENEMY_HIT,
    LOBBY_CREATE, LOBBY_JOIN_REQUEST, LOBBY_JOIN_RESPONSE, LOBBY_LIST_RESPONSE,
    PLAYER_READY, LOBBY_UPDATE, LOBBY_START, LOBBY_LIST_REQUEST, LOBBY_LEAVE,
    MESSAGE, LOBBY_KICK, BOSS_SPAWN, FORCE_STATE, CLIENT_LEAVE, MAP_DATA, LEVEL_TRANSITION>;

struct Event {
  EventType type;
  EventData data;
  uint16_t seqNum;
  uint16_t ack;
  uint32_t ack_bits;
};
