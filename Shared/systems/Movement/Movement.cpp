// Copyright 2025 Dalia Guiz
#include "./Movement.hpp"

#include <algorithm>
#include <iostream>

#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Zipper.hpp"
#include "inputs/InputManager.hpp"

void enemy_movement_system(SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Enemy>& enemies, float deltaTime) {
  for (auto&& [transform, rigidbody, enemy] :
       Zipper(transforms, rigidbodies, enemies)) {
    enemy.timer += deltaTime;

    switch (enemy.type) {
      case EnemyType::Basic:
        rigidbody.velocity.x = -enemy.speed;
        break;
      case EnemyType::Zigzag:
        rigidbody.velocity.x = -enemy.speed;
        rigidbody.velocity.y = std::sin(enemy.timer * 5.f) * enemy.amplitude;
        std::cout << enemy.timer;
        break;
      case EnemyType::Wave:
        rigidbody.velocity.x = -enemy.speed;
        rigidbody.velocity.y = std::cos(enemy.timer * 2.f) * enemy.amplitude;
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
      // registry knows entity id : convert index -> Entity
      registry.kill_entity(registry.entity_from_index(i));
    }
  }
}

void boss_movement_system(SparseArray<Transform>& transforms,
                          SparseArray<RigidBody>& rigidbodies,
                          SparseArray<Boss>& bosses, float deltaTime) {
  for (auto&& [transform, rigidbody, boss] :
       Zipper(transforms, rigidbodies, bosses)) {
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

      case BossType::Snake: {
        rigidbody.velocity.x = -boss.speed;
        rigidbody.velocity.y = std::sin(boss.timer * 8.f) * boss.amplitude;
      } break;

      case BossType::BydoEye: {
        rigidbody.velocity = {0.f, 0.f};
      } break;

      case BossType::Battleship: {
        rigidbody.velocity.x = -boss.speed * 0.5f;
        rigidbody.velocity.y = std::sin(boss.timer * 2.f) * 20.f;
      } break;

      case BossType::FinalBoss: {
        float t = boss.timer;
        float phase = fmod(t, 12.f);  // cycle de 12 secondes

        if (phase < 3.f) {
          rigidbody.velocity.x = -boss.speed * 1.2f;
          rigidbody.velocity.y = std::sin(t * 10.f) * (boss.amplitude * 1.5f);
        } else if (phase < 6.f) {
          float radius = 60.f;
          rigidbody.velocity.x = std::cos(t * 2.f) * radius;
          rigidbody.velocity.y = std::sin(t * 3.f) * radius;
        } else if (phase < 9.f) {
          float shake = std::sin(t * 30.f) * 15.f;
          rigidbody.velocity.x = -boss.speed * 3.f;
          rigidbody.velocity.y = shake;
        } else {
          rigidbody.velocity.x = boss.speed * 0.5f;
          rigidbody.velocity.y = std::sin(t * 1.5f) * 20.f;
        }
      } break;
    }
    // transform.position += rigidbody.velocity * deltaTime;
    if (boss.type == BossType::BigShip) {
      if (boss.timer > 10.f && boss.phase == BossPhase::Phase1)
        boss.phase = BossPhase::Phase2;

      if (boss.timer > 20.f && boss.phase == BossPhase::Phase2)
        boss.phase = BossPhase::Phase3;
    }
  }
}
