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
#include "ecs/Zipper.hpp"
#include "physics/Physics2D.hpp"
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
      case EnemyType::Basic: {
        float t = enemy.timer;

        // Zigzag vertical : va en haut puis en bas, oscillation sinusoïdale
        rigidbody.velocity.x = 0.0f;
        rigidbody.velocity.y = std::sin(t * 2.0f) * enemy.amplitude * 2.5f;

        // Tir (inchangé)
        if (enemy.timeSinceLastShot >= 1.5f) {
          Vector2 pos = transform.position + Vector2{-30.f, 0.f};
          spawn_projectile(registry, pos, {-1.f, 0.f}, 300.f, entityId);
          spawn_projectile(registry, pos, {-1.f, -0.3f}, 280.f, entityId);
          spawn_projectile(registry, pos, {-1.f, 0.3f}, 280.f, entityId);
          enemy.timeSinceLastShot = 0.f;
        }
        break;
      }

      case EnemyType::Zigzag: {
        float speedBoost = 1.f + std::abs(std::sin(enemy.timer * 2.f)) * 0.8f;
        rigidbody.velocity.x = -enemy.speed * speedBoost;

        float zigzag =
            std::sin(enemy.timer * 8.f) + std::sin(enemy.timer * 3.f) * 0.5f;
        rigidbody.velocity.y = zigzag * enemy.amplitude * 1.5f;

        if (closestPlayerPos.has_value() && fmod(enemy.timer, 3.f) < 0.5f) {
          float playerY = closestPlayerPos.value().y;
          float diff = playerY - transform.position.y;
          rigidbody.velocity.y += diff * 2.f;
        }

        if (transform.position.x <= -50.f) {
          transform.position.x = 850.f;
          static std::random_device rd;
          static std::mt19937 gen(rd());
          std::uniform_real_distribution<float> yDist(50.f, 550.f);
          transform.position.y = yDist(gen);
          enemy.timer = 0.f;
        }
        break;
      }

      case EnemyType::Chase: {
        if (closestPlayerPos.has_value()) {
          Vector2 toPlayer = closestPlayerPos.value() - transform.position;
          float distance = toPlayer.Length();

          if (distance > 0) {
            Vector2 dir = toPlayer.Normalized();

            if (distance > 300.f) {
              float spiral = enemy.timer * 4.f;
              rigidbody.velocity.x =
                  dir.x * enemy.speed + std::cos(spiral) * 80.f;
              rigidbody.velocity.y =
                  dir.y * enemy.speed + std::sin(spiral) * 80.f;
            } else if (distance > 100.f) {
              rigidbody.velocity.x = dir.x * enemy.speed * 1.8f;
              rigidbody.velocity.y = dir.y * enemy.speed * 1.8f;
            } else {
              if (fmod(enemy.timer, 2.f) < 0.8f) {
                rigidbody.velocity.x = -dir.x * enemy.speed * 0.5f;
                rigidbody.velocity.y = -dir.y * enemy.speed * 0.5f;
              } else {
                rigidbody.velocity.x = dir.x * enemy.speed * 2.5f;
                rigidbody.velocity.y = dir.y * enemy.speed * 2.5f;
              }
            }
          }
        } else {
          rigidbody.velocity.x = std::cos(enemy.timer * 2.f) * enemy.speed;
          rigidbody.velocity.y = std::sin(enemy.timer * 2.f) * enemy.speed;
        }
        break;
      }
      case EnemyType::mini_Green: {
        float cycle = fmod(enemy.timer, 4.f);

        if (cycle < 2.f) {
          rigidbody.velocity.x = 0.f;
          rigidbody.velocity.y = std::sin(enemy.timer * 3.f) * enemy.amplitude;
        } else if (cycle < 2.8f) {
          rigidbody.velocity.x = -enemy.speed * 3.f;
          rigidbody.velocity.y = 0.f;
        } else {
          rigidbody.velocity.x = enemy.speed;
          rigidbody.velocity.y = 0.f;
        }

        transform.position.x = std::clamp(transform.position.x, 150.f, 750.f);

        enemy.timeSinceLastShot += deltaTime;
        if (enemy.timeSinceLastShot >= 2.0f) {  // cooldown 2 secondes
          Vector2 pos =
              transform.position + Vector2{-20.f, 0.f};  // offset devant lui
          Vector2 dir = {-1.f, 0.f};                     // tirer vers la gauche
          float speed = 300.f;
          spawn_projectile(registry, pos, dir, speed, entityId);
          enemy.timeSinceLastShot = 0.f;
        }
        break;
      }

      case EnemyType::Spinner: {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        if (fmod(enemy.timer, 0.3f) < deltaTime) {
          std::uniform_real_distribution<float> yDist(-1.f, 1.f);
          enemy.direction.y = yDist(gen);
        }

        rigidbody.velocity.x = -enemy.speed * 2.f;
        rigidbody.velocity.y = enemy.direction.y * enemy.amplitude * 3.f;
        if (transform.position.x <= -50.f) {
          transform.position.x = 250.f;
          std::uniform_real_distribution<float> yPosDist(50.f, 250.f);
          transform.position.y = yPosDist(gen);
        }
      } break;
    }
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
          boss.direction.x = 1.f;
        }
        if (transform.position.x >= 650.0f) {
          boss.direction.x = -1.f;
        }

        rigidbody.velocity.y =
            std::sin(boss.timer * 2.f) * boss.amplitude * 5.f;

        const float PROJECTILE_FIRE_INTERVAL = 1.5f;  // plus rapide
        float timeMod = fmod(boss.timer, PROJECTILE_FIRE_INTERVAL);

        if (timeMod < 0.05f && timeMod > 0.0f) {
          spawn_boss_projectile(
              registry, {transform.position.x, transform.position.y - 40.f},
              entityId);
          spawn_boss_projectile(
              registry, {transform.position.x, transform.position.y}, entityId);
          spawn_boss_projectile(
              registry, {transform.position.x, transform.position.y + 40.f},
              entityId);

          boss.timer += 0.5f;
        }
      } break;

      case BossType::BydoEye: {
        rigidbody.velocity = {0.f, 0.f};
      } break;

      case BossType::Bydo_Battleship: {
        if (boss.direction.x == 0.f) {
          boss.direction.x = -1.f;
        }
        rigidbody.velocity.x = boss.direction.x * boss.speed * 0.3f;

        if (transform.position.x <= 400.0f) {
          boss.direction.x = 1.f;
        }
        if (transform.position.x >= 700.0f) {
          boss.direction.x = -1.f;
        }
        rigidbody.velocity.y = std::sin(boss.timer * 1.5f) * 15.f;
      } break;

      case BossType::FinalBoss: {
        float amplitude = 100.f;  // hauteur maximale du mouvement
        float speed = 2.f;        // vitesse du va-et-vient
        transform.position.y = 300.f + std::sin(boss.timer * speed) * amplitude;

        // Garde la position X fixe
        transform.position.x = 700.f;

        // Boss immobile horizontalement
        rigidbody.velocity = {0.f, 0.f};

        // Spawn d'ennemis comme avant
        if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
          std::cout << "Final Boss spawning Basic Enemy!" << std::endl;
          spawn_basic_enemy_for_boss(registry);
          enemySpawnTimer = 0.0f;
        }

        const float PROJECTILE_FIRE_INTERVAL = 2.0f;
        const float TIME_OFFSET = 0.2f;

        float timeMod = fmod(boss.timer, PROJECTILE_FIRE_INTERVAL);

        if (timeMod < 0.1f && timeMod > 0.0f) {
          const float Y_OFFSET_RANGE = 100.0f;  // plus visible que 70
          const int NUM_SHOTS = 5;

          for (int i = 0; i < NUM_SHOTS; ++i) {
            // variation verticale pour un tir en zigzag
            float yOffset = (i - NUM_SHOTS / 2) * 25.f +
                            std::sin(boss.timer * 3.f + i) * 20.f;
            spawn_boss_projectile(
                registry,
                {transform.position.x, transform.position.y + yOffset},
                entityId);
          }

          boss.timer += 0.5f;  // pour ne pas spammer trop vite
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

      part.timer += deltaTime;
      if (part.timer >= 1.5f) {
        Vector2 shootDir = {-1.f, 0.f};
        spawn_projectile(registry, partTransform.position, shootDir, 280.f, i);
        part.timer = 0.f;
      }
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

        float floatOffsetY = std::sin(floatTimer * 3.f) * 15.f;

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
        force.shootCooldown += deltaTime;
        if (force.shootCooldown >= 2.f) {
          spawn_boss_projectile(registry, transform.position,
                                0);  // ou l'ID du boss
          force.shootCooldown = 0.f;
        }
        break;
      }
    }
  }
}
