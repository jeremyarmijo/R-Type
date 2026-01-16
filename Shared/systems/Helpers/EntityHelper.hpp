#pragma once
#include <utility>
#include <vector>

#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/EnemySpawn.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/BossPart.hpp"
#include "components/Levels.hpp"
#include "physics/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "input/InputSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"

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

inline Entity createLevelEntity(Registry& registry,
                                const LevelComponent& level) {
  Entity lvlEntity = registry.spawn_entity();
  registry.add_component<LevelComponent>(lvlEntity, LevelComponent(level));

  return lvlEntity;
}

inline Entity createBossPart(Registry& registry, Entity bossEntity,
                             const Vector2& startPos, Vector2 offset,
                             int segmentIndex, float timeOffset, int hp,
                             Vector2 size = {30.f, 30.f}) {
  Entity part = registry.spawn_entity();
  registry.add_component<Transform>(part, Transform{startPos});
  registry.add_component<BoxCollider>(part, BoxCollider(size.x, size.y));
  registry.add_component<BossPart>(
      part, BossPart(bossEntity, offset, segmentIndex, timeOffset, hp));
  return part;
}

/**
 * @brief Creates a sprite
 *
 * @param textureKey texture key in the texture manager
 * @param position spawn coordinates
 * @param layer position sprite on different layers for rendering
 * @return Entity
 */
inline Entity CreateSprite(Registry& registry, const std::string& textureKey, Vector2 position,
                                int layer = 0) {
  Entity entity = registry.spawn_entity();

  registry.emplace_component<Transform>(entity, position, Vector2{1, 1},
                                          0.0f);
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
inline Entity CreatePhysicsObject(Registry& registry, const std::string& textureKey,
                                       Vector2 position, Vector2 size,
                                       bool isStatic, int layer) {
  Entity entity = CreateSprite(registry, textureKey, position, layer = 0);

  registry.emplace_component<RigidBody>(entity, 1.0f, 0.5f, isStatic);
  registry.emplace_component<BoxCollider>(entity, size.x, size.y);

  return entity;
}

inline Entity CreateAnimatedSprite(Registry& registry, const AnimationClip* clip, const std::string& textureKey,
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

inline Entity CreatePlayer(Registry& registry, const AnimationClip* clip, const std::string& textureKey,
                                const std::string& animationKey,
                                Vector2 position, float moveSpeed, int layer = 0) {
  Entity player = CreateAnimatedSprite(registry, clip, textureKey, position, animationKey, layer);

  registry.emplace_component<RigidBody>(player, 1.0f, 0.5f, false);
  registry.emplace_component<BoxCollider>(player, 60.0f, 32.0f);
  registry.emplace_component<PlayerEntity>(player, moveSpeed);

  registry.add_component<Weapon>(player, Weapon());

  return player;
}
