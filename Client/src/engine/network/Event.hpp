#pragma once
#include <variant>
#include <string>
#include <vector>
#include <cstdint>

// -------------------------
// TCP Messages
// -------------------------

struct LOGIN_REQUEST {
    uint8_t usernameLen;
    std::string username;
    uint8_t passwordLen;
    std::string password;
};

struct LOGIN_RESPONSE {
    uint8_t success;
    uint16_t playerId;
    uint32_t serverTick;
    uint16_t udpPort;
    uint16_t errorCode;
    std::string message;
};

struct SIGNUP_REQUEST {
    uint8_t usernameLen;
    std::string username;
    uint8_t passwordLen;
    std::string password;
    uint8_t emailLen;
    std::string email;
};

struct SIGNUP_RESPONSE {
    uint8_t success;
    std::string message;
    uint16_t errorCode;
};

struct LOGOUT {
    uint16_t playerId;
};

struct LOBBY_LIST_REQUEST {
    // Pas de payload
};

struct LOBBY_LIST_RESPONSE {
    struct Lobby {
        uint16_t lobbyId;
        std::string name;
        uint8_t playerCount;
        uint8_t maxPlayers;
        uint8_t hasStarted;
    };
    std::vector<Lobby> lobbies;
};

struct LOBBY_JOIN {
    uint16_t lobbyId;
};

struct LOBBY_UPDATE {
    struct PlayerInfo {
        uint16_t playerId;
        std::string name;
        uint8_t ready;
    };
    uint16_t lobbyId;
    std::vector<PlayerInfo> players;
};

struct PLAYER_READY {
    uint16_t playerId;
    uint8_t ready;
};

struct LOBBY_LEAVE {
    uint16_t playerId;
};

struct CHAT_MESSAGE {
    uint16_t senderId;
    std::string message;
};

struct GAME_LOADING {
    uint16_t mapId;
    uint8_t gameMode;
    uint8_t difficulty;
    uint32_t mapWidth;
    uint16_t mapHeight;
    uint16_t chunkSize;
    uint32_t tickStart;
};

struct PLAYER_END_LOADING {
    uint16_t playerId;
    uint16_t missingChunks;
};

struct GAME_START {
    uint32_t startTick;
    float playerSpawnX;
    float playerSpawnY;
    float scrollSpeed;
};

struct GAME_END {
    struct Score {
        uint16_t playerId;
        uint32_t score;
        uint16_t kills;
    };
    uint8_t victory;
    std::vector<Score> scores;
};

struct PLAYER_DISCONNECT {
    uint16_t playerId;
    uint8_t reason;
};

struct ERROR {
    uint16_t errorCode;
    std::string message;
};

struct CHUNK_REQUEST {
    int32_t chunkX;
};

struct CHUNK_DATA {
    struct Tile {
        uint16_t tileX;
        uint16_t tileY;
        uint8_t tileType;
        uint8_t tileSprite;
        uint8_t tileFlags;
        uint8_t tileHealth;
    };
    int32_t chunkX;
    uint16_t chunkWidth;
    uint16_t chunkHeight;
    std::vector<Tile> tiles;
};

// -------------------------
// UDP Messages
// -------------------------
struct PLAYER_INPUT {
    uint32_t sequenceNum;
    uint32_t tick;
    int8_t moveX;
    int8_t moveY;
    uint8_t actions;
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
        uint8_t pattern;
        int8_t direction;
        uint8_t flags;
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
    uint32_t sequenceNum;
    uint32_t tick;
    std::vector<PlayerState> players;
    std::vector<EnemyState> enemies;
    std::vector<ProjectileState> projectiles;
};

struct ENTITY_SPAWN {
    uint32_t sequenceNum;
    uint16_t entityId;
    uint8_t entityType;
    uint8_t subType;
    float posX;
    float posY;
};

struct ENTITY_DESTROY {
    uint32_t sequenceNum;
    uint16_t entityId;
    uint8_t destroyType;
    uint16_t killerId;
};

struct PLAYER_HIT {
    uint32_t sequenceNum;
    uint16_t playerId;
    uint8_t damage;
    uint16_t attackerId;
    uint8_t newHp;
};

struct POWERUP_COLLECTED {
    uint32_t sequenceNum;
    uint16_t playerId;
    uint16_t powerupId;
    uint8_t powerupType;
};

struct FORCE_UPDATE {
    uint32_t sequenceNum;
    uint16_t playerId;
    uint8_t state;
    float posX;
    float posY;
    uint8_t level;
};

struct BOSS_SPAWN {
    uint32_t sequenceNum;
    uint16_t bossId;
    uint8_t bossType;
    uint16_t maxHp;
    uint8_t phase;
};

struct BOSS_UPDATE {
    uint32_t sequenceNum;
    uint16_t bossId;
    float posX;
    float posY;
    uint16_t hp;
    uint8_t phase;
    uint8_t action;
};

struct SCROLLING_UPDATE {
    uint32_t sequenceNum;
    float scrollSpeed;
    float offsetX;
};

struct ACK {
    uint32_t sequenceNum;
    uint8_t messageType;
};

struct CHUNK_UNLOAD {
    uint32_t sequenceNum;
    int32_t chunkX;
};

struct CHUNK_TILE_UPDATE {
    uint32_t sequenceNum;
    int32_t chunkX;
    uint16_t tileX;
    uint16_t tileY;
    uint8_t newTileType;
    uint8_t newHealth;
};

struct CHUNK_VISIBILITY {
    uint32_t sequenceNum;
    std::vector<int32_t> chunks;
};

using EventData = std::variant<
    std::monostate,
    LOGIN_REQUEST,
    LOGIN_RESPONSE,
    SIGNUP_REQUEST,
    SIGNUP_RESPONSE,
    LOGOUT,
    LOBBY_LIST_REQUEST,
    LOBBY_LIST_RESPONSE,
    LOBBY_JOIN,
    LOBBY_UPDATE,
    PLAYER_READY,
    LOBBY_LEAVE,
    CHAT_MESSAGE,
    GAME_LOADING,
    PLAYER_END_LOADING,
    GAME_START,
    GAME_END,
    PLAYER_DISCONNECT,
    ERROR,
    CHUNK_REQUEST,
    CHUNK_DATA,
    PLAYER_INPUT,
    GAME_STATE,
    ENTITY_SPAWN,
    ENTITY_DESTROY,
    PLAYER_HIT,
    POWERUP_COLLECTED,
    FORCE_UPDATE,
    BOSS_SPAWN,
    BOSS_UPDATE,
    SCROLLING_UPDATE,
    ACK,
    CHUNK_UNLOAD,
    CHUNK_TILE_UPDATE,
    CHUNK_VISIBILITY
>;

struct Event {
    uint8_t type;
    EventData data;
};
