// Copyright 2025 Dalia Guiz
#include "./Movement.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "Helpers/EntityHelper.hpp"
#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Zipper.hpp"
#include "inputs/InputManager.hpp"
#include "systems/ProjectileSystem.hpp"

void player_movement_system(Registry& registry) {
  auto& rigidbodies = registry.get_components<RigidBody>();
  auto& players = registry.get_components<PlayerEntity>();
  auto& states = registry.get_components<InputState>();

  for (auto&& [state, rb, player] : Zipper(states, rigidbodies, players)) {
    rb.velocity = {0.f, 0.f};
    if (state.moveLeft) rb.velocity.x = -player.speed;
    if (state.moveRight) rb.velocity.x = player.speed;
    if (state.moveUp) rb.velocity.y = -player.speed;
    if (state.moveDown) rb.velocity.y = player.speed;
  }
}

void enemy_movement_system(Registry& registry,
                           SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Enemy>& enemies,
                           SparseArray<PlayerEntity>& players,
                           float deltaTime) {
  std::optional<Vector2> closestPlayerPos = std::nullopt;

  for (auto&& [entityId, p_transform, p_player] :
       IndexedZipper(transforms, players)) {
    if (p_player.isAlive) {
      closestPlayerPos = p_transform.position;
      break;
    }
  }
  for (auto&& [entityId, transform, rigidbody, enemy] :
       IndexedZipper(transforms, rigidbodies, enemies)) {
    enemy.timer += deltaTime;
    enemy.timeSinceLastShot += deltaTime;
    switch (enemy.type) {
      case EnemyType::Basic:
        rigidbody.velocity.x = 0.0f;
        rigidbody.velocity.y = std::sin(enemy.timer * 1.5f) * enemy.amplitude;
        if (enemy.timeSinceLastShot >= 1) {
          Vector2 spawnPosition =
              transform.position + Vector2{-enemy.amplitude, 0.0f};
          Vector2 direction = {-1.0f, 0.0f};
          spawn_projectile(registry, spawnPosition, direction, 300.f, entityId);
          enemy.timeSinceLastShot = 0.0f;
        }
        break;
      case EnemyType::Zigzag:
        rigidbody.velocity.x = -enemy.speed;
        rigidbody.velocity.y = std::sin(enemy.timer * 5.f) * enemy.amplitude;
        if (transform.position.x <= 0.0f) {
          transform.position.x = 800 + 50.0f;
          enemy.timer = 0.0f;
        }
        break;
      case EnemyType::Chase:
        if (closestPlayerPos.has_value()) {
          Vector2 direction = closestPlayerPos.value() - transform.position;

          if (direction.Length() > 0) {
            Vector2 velocity = direction.Normalized() * enemy.speed;

            rigidbody.velocity.x = velocity.x;
            rigidbody.velocity.y = velocity.y;
          } else {
            rigidbody.velocity.x = 0.0f;
            rigidbody.velocity.y = 0.0f;
          }
        } else {
          rigidbody.velocity.x = -enemy.speed / 2.0f;
          rigidbody.velocity.y = 0.0f;
        }
        break;
    }
    // transform.position += rigidbody.velocity * deltaTime;
  }
}

void Projectile_movement_system(SparseArray<Transform>& transforms,
                                SparseArray<RigidBody>& rigidbodies,
                                SparseArray<Projectile>& projectiles,
                                Registry& registry, float deltaTime) {
  size_t max =
      std::max({transforms.size(), rigidbodies.size(), projectiles.size()});
  for (size_t i = 0; i < max; ++i) {
    if (!projectiles[i].has_value()) continue;
    if (!transforms[i].has_value()) continue;
    if (!rigidbodies[i].has_value()) continue;

    auto& transform = transforms[i].value();
    auto& rigidbody = rigidbodies[i].value();
    auto& proj = projectiles[i].value();

    proj.currentLife += deltaTime;
    transform.position += rigidbody.velocity * deltaTime;

    if (proj.currentLife > 2.f) {
      registry.kill_entity(registry.entity_from_index(i));
    }
  }
}

void spawn_basic_enemy_for_boss(Registry& registry) {
  const float MIN_Y = 30.0f;
  const float MAX_Y = 550.0f;
  const float SPAWN_X = 750.0f;

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(MIN_Y, MAX_Y);

  float spawnY = dis(gen);

  Vector2 pos = {SPAWN_X, spawnY};
  createEnemy(registry, EnemyType::Basic, pos);
}

void spawn_boss_projectile(Registry& registry, Vector2 position,
                           size_t bossEntityId) {
  Vector2 direction = {-1.0f, 0.0f};
  const float PROJECTILE_SPEED = 350.0f;
  spawn_projectile(registry, position, direction, PROJECTILE_SPEED,
                   bossEntityId);
}

void boss_movement_system(Registry& registry,
                          SparseArray<Transform>& transforms,
                          SparseArray<RigidBody>& rigidbodies,
                          SparseArray<Boss>& bosses, float deltaTime) {
  static float enemySpawnTimer = 0.0f;
  const float ENEMY_SPAWN_INTERVAL = 5.0f;

  for (auto&& [entityId, transform, rigidbody, boss] :
       IndexedZipper(transforms, rigidbodies, bosses)) {
    boss.timer += deltaTime;

    switch (boss.type) {
      case BossType::BigShip: {
        switch (boss.phase) {
          case BossPhase::Phase1:
            rigidbody.velocity.x = std::sin(boss.timer * 1.5f) * boss.speed;
            break;

          case BossPhase::Phase2:
            rigidbody.velocity.y = std::sin(boss.timer * 3.f) * boss.amplitude;
            break;

          case BossPhase::Phase3:
            rigidbody.velocity.x = -boss.speed * 1.2f;
            rigidbody.velocity.y =
                std::sin(boss.timer * 5.f) * boss.amplitude * 1.5f;
            break;
        }
      } break;

      case BossType::Gomander_snake: {
        if (boss.direction.x == 0.f) {
          boss.direction.x = -1.f;
        }
        rigidbody.velocity.x = boss.direction.x * boss.speed;

        if (transform.position.x <= 200.0f) {
          boss.direction.x = 1.f;  // Repart à droite
        }
        if (transform.position.x >= 650.0f) {
          boss.direction.x = -1.f;  // Repart à gauche
        }
        rigidbody.velocity.y =
            std::sin(boss.timer * 3.f) * boss.amplitude * 3.f;
      } break;

      case BossType::BydoEye: {
        rigidbody.velocity = {0.f, 0.f};
      } break;

      case BossType::Bydo_Battleship: {
        if (boss.direction.x == 0.f) {
          boss.direction.x = -1.f;  // Commence par aller à gauche
        }
        rigidbody.velocity.x =
            boss.direction.x * boss.speed * 0.3f;  // Très lent

        if (transform.position.x <= 400.0f) {
          boss.direction.x = 1.f;  // Repart à droite
        }
        if (transform.position.x >= 700.0f) {
          boss.direction.x = -1.f;  // Repart à gauche
        }
        rigidbody.velocity.y =
            std::sin(boss.timer * 1.5f) * 15.f;  // Lent et léger
      } break;

      case BossType::FinalBoss: {
        rigidbody.velocity = {0.f, 0.f};
        transform.position.x = 700.0f;
        transform.position.y = 300.0f;

        if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
          std::cout << "Final Boss spawning Basic Enemy!" << std::endl;
          spawn_basic_enemy_for_boss(registry);
          enemySpawnTimer = 0.0f;
        }

        const float PROJECTILE_FIRE_INTERVAL = 2.0f;
        const float TIME_OFFSET = 0.2f;

        float timeMod = fmod(boss.timer, PROJECTILE_FIRE_INTERVAL);

        if (timeMod < 0.1f && timeMod > 0.0f) {
          const float Y_OFFSET_RANGE = 70.0f;
          float y1 = transform.position.y - Y_OFFSET_RANGE;
          spawn_boss_projectile(registry, {transform.position.x, y1}, entityId);
          float y2 = transform.position.y;
          spawn_boss_projectile(registry, {transform.position.x - 10.0f, y2},
                                entityId);
          float y3 = transform.position.y + Y_OFFSET_RANGE;
          spawn_boss_projectile(registry, {transform.position.x, y3}, entityId);
          boss.timer += 0.5f;
        }
      } break;
    }
    if (boss.type == BossType::BigShip) {
      if (boss.timer > 10.f && boss.phase == BossPhase::Phase1)
        boss.phase = BossPhase::Phase2;

      if (boss.timer > 20.f && boss.phase == BossPhase::Phase2)
        boss.phase = BossPhase::Phase3;
    }
  }
}

void boss_part_system(Registry& registry, float deltaTime) {
  auto& parts = registry.get_components<BossPart>();
  auto& transforms = registry.get_components<Transform>();
  auto& bosses = registry.get_components<Boss>();

  for (size_t i = 0; i < parts.size(); ++i) {
    if (!parts[i].has_value()) continue;
    if (i >= transforms.size() || !transforms[i].has_value()) continue;

    BossPart& part = parts[i].value();
    if (!part.alive) continue;

    Transform& partTransform = transforms[i].value();

    size_t bossId = static_cast<size_t>(part.bossEntity);
    if (bossId >= bosses.size() || !bosses[bossId].has_value()) continue;
    if (bossId >= transforms.size() || !transforms[bossId].has_value())
      continue;

    Boss& boss = bosses[bossId].value();
    Transform& bossTransform = transforms[bossId].value();

    if (part.segmentIndex >= 0) {
      float delayedTimer = boss.timer - part.timeOffset;
      float offsetX = (part.segmentIndex + 1) * 35.f;

      partTransform.position.x =
          bossTransform.position.x + (boss.direction.x * -1.f) * offsetX;
      partTransform.position.y =
          bossTransform.position.y +
          std::sin(delayedTimer * 5.f) * boss.amplitude * 1.5f;

    } else {
      partTransform.position.x = bossTransform.position.x + part.offset.x;
      partTransform.position.y = bossTransform.position.y + part.offset.y;
    }
  }
}
void force_movement_system(Registry& registry,
                           SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Force>& forces,
                           SparseArray<PlayerEntity>& players,
                           float deltaTime) {
  for (auto&& [forceIdx, transform, rigidbody, force] :
       IndexedZipper(transforms, rigidbodies, forces)) {
    if (!force.isActive) continue;

    size_t playerEntityId = static_cast<size_t>(force.ownerPlayer);

    std::cout << "[FORCE] forceIdx=" << forceIdx
              << " ownerEntity=" << playerEntityId
              << " state=" << static_cast<int>(force.state) << std::endl;

    if (playerEntityId >= transforms.size() ||
        !transforms[playerEntityId].has_value()) {
      std::cout << "[FORCE] Player transform NOT FOUND!" << std::endl;
      continue;
    }
    if (playerEntityId >= players.size() ||
        !players[playerEntityId].has_value()) {
      continue;
    }
    if (!players[playerEntityId]->isAlive) {
      continue;
    }

    Vector2 playerPos = transforms[playerEntityId]->position;

    switch (force.state) {
      case EForceState::AttachedFront: {
        static float floatTimer = 0.f;
        floatTimer += deltaTime;

        float floatOffsetY =
            std::sin(floatTimer * 3.f) * 15.f;  // Oscille de ±15 pixels

        transform.position.x = playerPos.x + force.offsetFront.x;
        transform.position.y = playerPos.y + force.offsetFront.y;
        rigidbody.velocity = {0.f, 0.f};
        break;
      }

      case EForceState::AttachedBack: {
        transform.position.x = playerPos.x + force.offsetBack.x;
        transform.position.y = playerPos.y + force.offsetBack.y;
        rigidbody.velocity = {0.f, 0.f};
        break;
      }

      case EForceState::Detached: {
        rigidbody.velocity.x = force.direction.x * force.speed;
        rigidbody.velocity.y = force.direction.y * force.speed;

        force.currentDistance += force.speed * deltaTime;

        if (force.currentDistance >= force.maxDistance) {
          rigidbody.velocity = {0.f, 0.f};
        }
        if (transform.position.x >= 750.f || transform.position.x <= 50.f) {
          rigidbody.velocity = {0.f, 0.f};
        }
        break;
      }
    }
  }
}
