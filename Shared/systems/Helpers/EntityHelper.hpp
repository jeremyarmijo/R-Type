#pragma once
#include <utility>
#include <vector>

#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/EnemySpawn.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "inputs/InputManager.hpp"

static const Vector2 PLAYER_SIZE{32.f, 32.f};
static const Vector2 ENEMY_BASIC_SIZE{32.f, 32.f};
static const Vector2 ENEMY_BOSS_SIZE{128.f, 128.f};

enum class EntityType { Player, Enemy, Boss, Projectile };

// Helper pour créer un joueur
inline Entity createPlayer(Registry& registry, const Vector2& startPos,
                           int playerId = 0) {
  Entity player = registry.spawn_entity();

  registry.add_component<Transform>(player, Transform{startPos});
  registry.add_component<RigidBody>(player, RigidBody{});
  registry.add_component<InputState>(player, InputState());
  registry.add_component<BoxCollider>(
      player, BoxCollider(PLAYER_SIZE.x, PLAYER_SIZE.y));
  registry.add_component<PlayerEntity>(player,
                                       PlayerEntity(playerId, 200.f, 100, 100));

  return player;
}

// Helper pour créer un ennemi basique
inline Entity createEnemy(Registry& registry, EnemyType type,
                          const Vector2& startPos) {
  static int enemyIdCounter = 0;
  Entity enemy = registry.spawn_entity();

  registry.add_component<Transform>(enemy, Transform{startPos});
  registry.add_component<RigidBody>(enemy, RigidBody{});
  registry.add_component<BoxCollider>(
      enemy, BoxCollider(ENEMY_BASIC_SIZE.x, ENEMY_BASIC_SIZE.y));
  registry.add_component<Enemy>(enemy, Enemy{type, 150.f});
  return enemy;
}

// Helper pour créer un boss
inline Entity createBoss(Registry& registry, BossType type,
                        const Vector2& startPos,
                        BossPhase phase, int maxHp) {
    Entity boss = registry.spawn_entity();
    registry.add_component<Transform>(boss, Transform{startPos});
    registry.add_component<RigidBody>(boss, RigidBody{});
    registry.add_component<BoxCollider>(boss, BoxCollider(ENEMY_BOSS_SIZE.x,
    ENEMY_BOSS_SIZE.y));
    registry.add_component<Boss>(boss, Boss(type, phase, 100.f, {0, 0}
    , 40.f, maxHp));
    return boss;
}


inline Entity createProjectile(Registry& registry, const Vector2& startPos,
                               const Vector2& direction, float speed,
                               Uint32 damage, bool fromPlayer) {
  Entity projectile = registry.spawn_entity();

  registry.add_component<Transform>(projectile, Transform(startPos));
  registry.add_component<RigidBody>(projectile, RigidBody());
  registry.add_component<BoxCollider>(projectile, BoxCollider(10.f, 10.f));
  registry.add_component<Projectile>(
      projectile, Projectile(damage, speed, direction.Normalized(), 5.0f,
                             fromPlayer ? 1 : 0));

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
