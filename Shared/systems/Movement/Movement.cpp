// Copyright 2025 Dalia Guiz
#include "./Movement.hpp"

#include <algorithm>

#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerController.hpp"
#include "Player/ProjectTile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Zipper.hpp"
#include "inputs/InputManager.hpp"

void player_movement_system(Registry& registry,
                            SparseArray<PlayerControlled>& playerControlled,
                            SparseArray<Transform>& transforms,
                            SparseArray<RigidBody>& rigidbodies,
                            SparseArray<InputState>& inputStates,
                            float deltaTime) {
  for (auto&& [playerComp, transform, rigidbody, input] :
       Zipper(playerControlled, transforms, rigidbodies, inputStates)) {
    if (rigidbody.isStatic) continue;
    rigidbody.velocity = {0.f, 0.f};

    if (input.moveLeft) {
      rigidbody.velocity.x = -playerComp.moveSpeed;
    }
    if (input.moveRight) {
      rigidbody.velocity.x = playerComp.moveSpeed;
    }
    if (input.moveUp) {
      rigidbody.velocity.y = -playerComp.moveSpeed;
    }
    if (input.moveDown) {
      rigidbody.velocity.y = playerComp.moveSpeed;
    }
    transform.position += rigidbody.velocity * deltaTime;
  }
}

void enemy_movement_system(SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Enemy>& enemies, float deltaTime) {
  for (auto&& [transform, rigidbody, enemy] :
       Zipper(transforms, rigidbodies, enemies)) {
    enemy.timer += deltaTime;  // l'utiliser pour le mouvement enemy
    rigidbody.velocity = {0.f, 0.f};
    switch (enemy.type) {
      case EnemyType::Basic:
        rigidbody.velocity.x = -enemy.speed;
        break;
      case EnemyType::Zigzag:
        rigidbody.velocity.x = -enemy.speed;
        rigidbody.velocity.y = std::sin(enemy.timer * 5.f) * enemy.amplitude;
        break;
      case EnemyType::Wave:
        rigidbody.velocity.x = -enemy.speed;
        rigidbody.velocity.y = std::cos(enemy.timer * 2.f) * enemy.amplitude;
        break;
    }
    transform.position += rigidbody.velocity * deltaTime;
  }
}

void projectTile_movement_system(SparseArray<Transform>& transforms,
                                 SparseArray<RigidBody>& rigidbodies,
                                 SparseArray<ProjectTile>& projectiles,
                                 Registry& registry, float deltaTime) {
  size_t max =
      std::max({transforms.size(), rigidbodies.size(), projectiles.size()});
  // Vérifie si l’entité possède effectivement les 3 composants
  for (size_t i = 0; i < max; ++i) {
    if (!projectiles[i].has_value()) continue;
    if (!transforms[i].has_value()) continue;
    if (!rigidbodies[i].has_value()) continue;

    auto& transform = transforms[i].value();
    auto& rigidbody = rigidbodies[i].value();
    auto& proj =
        projectiles[i].value();  // je change car le zipper le fait deja

    proj.timer += deltaTime;
    transform.position += rigidbody.velocity * deltaTime;
    // Suppression automatique après 2 secondes (éviter projectiles infinis)
    if (proj.timer > 2.f) {
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
    rigidbody.velocity = {0.f, 0.f};

    switch (boss.type) {
      case BossType::BigShip: {
        switch (boss.phase) {
          case BossPhase::Phase1:
            //  Oscillation horizontale lente
            rigidbody.velocity.x = std::sin(boss.timer * 1.5f) * boss.speed;
            break;

          case BossPhase::Phase2:
            // Oscillation verticale
            rigidbody.velocity.y = std::sin(boss.timer * 3.f) * boss.amplitude;
            break;

          case BossPhase::Phase3:
            // Mouvement combiné + accéléré
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
          // Attaque 1 : mouvements rapides + oscillation forte
          rigidbody.velocity.x = -boss.speed * 1.2f;
          rigidbody.velocity.y = std::sin(t * 10.f) * (boss.amplitude * 1.5f);
        } else if (phase < 6.f) {
          // Attaque 2 : mouvement circulaire
          float radius = 60.f;
          rigidbody.velocity.x = std::cos(t * 2.f) * radius;
          rigidbody.velocity.y = std::sin(t * 3.f) * radius;
        } else if (phase < 9.f) {
          // Attaque 3 : dash rapide + tremblement
          float shake = std::sin(t * 30.f) * 15.f;
          rigidbody.velocity.x = -boss.speed * 3.f;
          rigidbody.velocity.y = shake;
        } else {
          // Attaque 4 : mouvement lent oscillant
          rigidbody.velocity.x = boss.speed * 0.5f;
          rigidbody.velocity.y = std::sin(t * 1.5f) * 20.f;
        }
      } break;  // envoyer l'id de la photo
    }
    transform.position += rigidbody.velocity * deltaTime;
    // Gestion automatique des phases du BigShip
    if (boss.type == BossType::BigShip) {
      if (boss.timer > 10.f && boss.phase == BossPhase::Phase1)
        boss.phase = BossPhase::Phase2;

      if (boss.timer > 20.f && boss.phase == BossPhase::Phase2)
        boss.phase = BossPhase::Phase3;
    }
  }
}
