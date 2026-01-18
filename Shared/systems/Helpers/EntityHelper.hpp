#pragma once
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/EnemySpawn.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/BossPart.hpp"
#include "components/Force.hpp"
#include "components/Levels.hpp"
#include "components/TileMap.hpp"
#include "ecs/Registry.hpp"
#include "input/InputSubsystem.hpp"
#include "physics/Physics2D.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "stdlib.h"

static const Vector2 PLAYER_SIZE{32.f, 32.f};
static const Vector2 ENEMY_BASIC_SIZE{40.f, 40.f};
static const Vector2 ENEMY_BOSS_SIZE{128.f, 240.f};

enum class EntityType { Player, Enemy, Boss, Projectile };

// Helper pour créer un joueur
inline Entity createPlayer(Registry& registry, const Vector2& startPos,
                           int playerId = 0) {
  Entity player = registry.spawn_entity();

  registry.add_component<Transform>(player, Transform{startPos});
  registry.add_component<RigidBody>(player, RigidBody{});
  registry.add_component<InputState>(player, InputState());
  registry.add_component<Weapon>(player, Weapon());
  registry.add_component<BoxCollider>(
      player, BoxCollider(PLAYER_SIZE.x, PLAYER_SIZE.y));
  registry.add_component<PlayerEntity>(player,
                                       PlayerEntity(playerId, 200.f, 100, 100));

  return player;
}

// Helper pour créer un ennemi basique
// Helper pour créer un ennemi avec stats configurables
inline Entity createEnemy(Registry& registry, EnemyType type,
                          const Vector2& startPos, float speedMultiplier = 1.0f,
                          float hpMultiplier = 1.0f) {
  Entity enemy = registry.spawn_entity();

  // Stats de base selon le type
  float baseSpeed = 150.f;
  int baseHp = 50;
  int score = 100;
  int damage = 10;

  switch (type) {
    case EnemyType::Basic:
      baseSpeed = 100.f;
      baseHp = 50;
      score = 100;
      damage = 10;
      break;
    case EnemyType::Zigzag:
      baseSpeed = 120.f;
      baseHp = 40;
      score = 150;
      damage = 10;
      break;
    case EnemyType::Chase:
      baseSpeed = 80.f;
      baseHp = 60;
      score = 200;
      damage = 15;
      break;
    case EnemyType::mini_Green:
      baseSpeed = 150.f;
      baseHp = 30;
      score = 75;
      damage = 5;
      break;
    case EnemyType::Spinner:
      baseSpeed = 60.f;
      baseHp = 80;
      score = 250;
      damage = 20;
      break;
    default:
      baseSpeed = 100.f;
      baseHp = 50;
      score = 100;
      damage = 10;
      break;
  }

  // Appliquer les multiplicateurs
  float finalSpeed = baseSpeed * speedMultiplier;
  int finalHp = static_cast<int>(baseHp * hpMultiplier);

  registry.add_component<Transform>(enemy, Transform{startPos});
  registry.add_component<RigidBody>(enemy, RigidBody{});
  registry.add_component<BoxCollider>(
      enemy, BoxCollider(ENEMY_BASIC_SIZE.x, ENEMY_BASIC_SIZE.y));
  registry.add_component<Enemy>(
      enemy, Enemy{type, finalSpeed, {-1, 0}, 80.f, finalHp, score, 0, damage});

  return enemy;
}
// Helper pour créer un boss
inline Entity createBoss(Registry& registry, BossType type,
                         const Vector2& startPos, BossPhase phase, int maxHp) {
  Entity boss = registry.spawn_entity();
  registry.add_component<Transform>(boss, Transform{startPos});
  registry.add_component<RigidBody>(boss, RigidBody{});
  registry.add_component<BoxCollider>(
      boss, BoxCollider(ENEMY_BOSS_SIZE.x, ENEMY_BOSS_SIZE.y));
  registry.add_component<Boss>(boss,
                               Boss(type, phase, 100.f, {0, 0}, 40.f, maxHp));
  return boss;
}

inline Entity createProjectile(Registry& registry, const Vector2& startPos,
                               const Vector2& direction, float speed,
                               Uint32 damage, bool fromPlayer,
                               int chargeLevel = 0) {  // ← AJOUTE
  Entity projectile = registry.spawn_entity();

  // Ajuste les stats selon le niveau de charge
  float finalDamage = damage;
  float colliderSize = 10.f;

  if (chargeLevel >= 1) {
    finalDamage = damage * 2.5f;  // 25 damage
    colliderSize = 20.f;
  }
  if (chargeLevel >= 2) {
    finalDamage = damage * 5.0f;  // 50 damage
    colliderSize = 35.f;
  }
  if (chargeLevel >= 3) {
    finalDamage = damage * 10.0f;  // 100 damage
    colliderSize = 50.f;
  }

  registry.add_component<Transform>(projectile, Transform(startPos));
  registry.add_component<RigidBody>(projectile, RigidBody());
  registry.add_component<BoxCollider>(projectile,
                                      BoxCollider(colliderSize, colliderSize));

  Projectile proj(finalDamage, speed, direction.Normalized(), 5.0f,
                  fromPlayer ? 1 : 0, chargeLevel);
  registry.add_component<Projectile>(projectile, std::move(proj));
  std::cout << "[PROJECTILE] Created with chargeLevel=" << chargeLevel
            << " damage=" << finalDamage << " size=" << colliderSize
            << std::endl;

  return projectile;
}

inline Entity createEnemySpawner(Registry& registry,
                                 std::vector<Vector2> points, float interval,
                                 int maxEnemies, EnemyType type) {
  Entity spawner = registry.spawn_entity();

  registry.add_component<EnemySpawning>(
      spawner, EnemySpawning(std::move(points), interval, maxEnemies, type));

  return spawner;
}

inline Entity createLevelEntity(Registry& registry,
                                const LevelComponent& level) {
  Entity lvlEntity = registry.spawn_entity();
  registry.add_component<LevelComponent>(lvlEntity, LevelComponent(level));

  return lvlEntity;
}

inline Entity createBossPart(Registry& registry, Entity bossEntity,
                             const Vector2& startPos, Vector2 offset,
                             int segmentIndex, float timeOffset, int hp,
                             Vector2 size = {30.f, 30.f},
                             uint8_t partType = 0) {
  Entity part = registry.spawn_entity();
  registry.add_component<Transform>(part, Transform{startPos});
  registry.add_component<BoxCollider>(part, BoxCollider(size.x, size.y));
  registry.add_component<BossPart>(
      part,
      BossPart(bossEntity, offset, segmentIndex, timeOffset, hp, partType));
  return part;
}

inline Entity createForce(Registry& registry, Entity playerEntity,
                          const Vector2& startPos) {
  Entity force = registry.spawn_entity();

  Force forceComponent(playerEntity);
  Vector2 forcePos = {startPos.x + forceComponent.offsetFront.x,
                      startPos.y + forceComponent.offsetFront.y};

  registry.add_component<Transform>(force, Transform{forcePos});
  registry.add_component<RigidBody>(force, RigidBody{});
  registry.add_component<BoxCollider>(force, BoxCollider(24.f, 24.f));
  registry.add_component<Force>(force, std::move(forceComponent));

  return force;
}

inline Entity createMapEntity(Registry& registry, uint16_t mapWidth,
                              uint16_t mapHeight, float scrollSpeed,
                              const std::vector<uint8_t>& tiles,
                              uint16_t tileSize = 32) {
  Entity mapEntity = registry.spawn_entity();

  TileMap tileMap;
  tileMap.width = mapWidth;
  tileMap.height = mapHeight;
  tileMap.tileSize = tileSize;
  tileMap.scrollSpeed = scrollSpeed;
  tileMap.scrollOffset = 0.0f;
  tileMap.tiles = tiles;

  registry.add_component<TileMap>(mapEntity, std::move(tileMap));

  std::cout << "[MAP] Created map entity: " << mapWidth << "x" << mapHeight
            << " tiles, scrollSpeed=" << scrollSpeed << std::endl;

  return mapEntity;
}

inline TileMap generateSimpleMap(int levelIndex, uint16_t screenWidth = 800,
                                 uint16_t screenHeight = 600) {
  TileMap map;
  const uint16_t tileSize = 32;
  const uint16_t mapWidthScreens = 8;
  uint16_t mapWidth = (screenWidth / tileSize) * mapWidthScreens;
  uint16_t mapHeight = screenHeight / tileSize;

  map.init(mapWidth, mapHeight, tileSize);
  map.scrollSpeed = 50.0f + (levelIndex * 10.0f);

  for (int x = 0; x < mapWidth; ++x) {
    map.setTile(x, 0, TileType::CEILING);
    map.setTile(x, 1, TileType::CEILING);
  }

  int groundY = mapHeight - 2;
  for (int x = 0; x < mapWidth; ++x) {
    map.setTile(x, groundY, TileType::GROUND);
    map.setTile(x, groundY + 1, TileType::GROUND);
  }

  std::cout << "[MapGenerator] Creating map " << mapWidth << "x" << mapHeight
            << " for level " << levelIndex << std::endl;

  if (levelIndex >= 1) {
    srand(static_cast<unsigned>(levelIndex * 12345));
    int numHoles = 3 + levelIndex * 2;
    for (int i = 0; i < numHoles; ++i) {
      int holeX = 30 + (rand() % (mapWidth - 60));
      int holeWidth = 2 + (rand() % 3);
      for (int dx = 0; dx < holeWidth; ++dx) {
        map.setTile(holeX + dx, groundY, TileType::EMPTY);
        map.setTile(holeX + dx, groundY + 1, TileType::EMPTY);
      }
    }
  }

  if (levelIndex >= 1) {
    int numPlatforms = 2 + levelIndex;
    for (int i = 0; i < numPlatforms; ++i) {
      int platX = 40 + (rand() % (mapWidth - 80));
      int platY = 5 + (rand() % (mapHeight - 10));
      int platWidth = 3 + (rand() % 4);
      for (int dx = 0; dx < platWidth; ++dx) {
        map.setTile(platX + dx, platY, TileType::PLATFORM);
      }
    }
  }

  std::cout << "[MapGenerator] Map generated with " << map.tiles.size()
            << " tiles (ceiling + ground)" << std::endl;

  return map;
}
/**
 * @brief Creates a sprite
 *
 * @param textureKey texture key in the texture manager
 * @param position spawn coordinates
 * @param layer position sprite on different layers for rendering
 * @return Entity
 */
inline Entity CreateSprite(Registry& registry, const std::string& textureKey,
                           Vector2 position, int layer = 0) {
  Entity entity = registry.spawn_entity();

  registry.emplace_component<Transform>(entity, position, Vector2{1, 1}, 0.0f);
  registry.emplace_component<Sprite>(entity, textureKey, SDL_Rect{0, 0, 0, 0},
                                     Vector2{0.5f, 0.5f}, layer);
  return entity;
}

/**
 * @brief Creates a sprite with physics components; rigidBody and boxCollider
 *
 * @param textureKey texture key in the texture manager
 * @param position spawn coordinates
 * @param size size of the box collider
 * @param isStatic
 * @return Entity
 */
inline Entity CreatePhysicsObject(Registry& registry,
                                  const std::string& textureKey,
                                  Vector2 position, Vector2 size, bool isStatic,
                                  int layer) {
  Entity entity = CreateSprite(registry, textureKey, position, layer = 0);

  registry.emplace_component<RigidBody>(entity, 1.0f, 0.5f, isStatic);
  registry.emplace_component<BoxCollider>(entity, size.x, size.y);

  return entity;
}

inline Entity CreateAnimatedSprite(Registry& registry,
                                   const AnimationClip* clip,
                                   const std::string& textureKey,
                                   Vector2 position,
                                   const std::string& animationKey,
                                   int layer = 0) {
  Entity entity = CreateSprite(registry, textureKey, position, layer);

  registry.emplace_component<Animation>(entity, animationKey, true);
  auto& sprite = registry.get_components<Sprite>()[entity];
  auto& animation = registry.get_components<Animation>()[entity];

  if (clip && !clip->frames.empty()) {
    animation->currentFrame = 0;
    animation->currentTime = 0;
    animation->isPlaying = true;

    sprite->sourceRect = clip->frames[0].sourceRect;
  } else {
    std::cerr << "CreateAnimatedSprite ERROR: Animation not found: "
              << animationKey << std::endl;
  }

  return entity;
}

inline Entity CreatePlayer(Registry& registry, const AnimationClip* clip,
                           const std::string& textureKey,
                           const std::string& animationKey, Vector2 position,
                           float moveSpeed, int layer = 0) {
  Entity player = CreateAnimatedSprite(registry, clip, textureKey, position,
                                       animationKey, layer);

  registry.emplace_component<RigidBody>(player, 1.0f, 0.5f, false);
  registry.emplace_component<BoxCollider>(player, 60.0f, 32.0f);
  registry.emplace_component<PlayerEntity>(player, moveSpeed);

  registry.add_component<Weapon>(player, Weapon());

  return player;
}
