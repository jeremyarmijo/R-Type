#include "systems/ForceCtrl.hpp"
#include <iostream>
#include "components/BossPart.hpp"
#include "components/Player/Enemy.hpp"
#include "systems/Collision/Collision.hpp"

void force_control_system(Registry& registry, SparseArray<Force>& forces,
                          SparseArray<InputState>& states,
                          SparseArray<Transform>& transforms) {
  for (auto&& [forceIdx, force] : IndexedZipper(forces)) {
    if (!force.isActive) continue;

    size_t playerId = static_cast<size_t>(force.ownerPlayer);

    if (playerId >= states.size() || !states[playerId].has_value()) continue;
    if (playerId >= transforms.size() || !transforms[playerId].has_value())
      continue;

    InputState& input = states[playerId].value();

    if (input.action2) {
      if (force.state == EForceState::AttachedFront ||
          force.state == EForceState::AttachedBack) {
        force.state = EForceState::Detached;
        force.detachPosition = transforms[forceIdx]->position;
        force.currentDistance = 0.f;
        std::cout << "Force DETACHED!" << std::endl;
      } else {
        force.state = EForceState::AttachedFront;
        force.currentDistance = 0.f;
        std::cout << "Force RECALLED!" << std::endl;
      }
      input.action2 = false;
    }
  }
}

void force_collision_system(
    Registry& registry, SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders, SparseArray<Force>& forces,
    SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
    SparseArray<BossPart>& bossParts, SparseArray<Projectile>& projectiles) {
  for (auto&& [forceIdx, forceTransform, forceCollider, force] :
       IndexedZipper(transforms, colliders, forces)) {
    if (!force.isActive) continue;

    for (auto&& [enemyIdx, enemyTransform, enemyCollider, enemy] :
         IndexedZipper(transforms, colliders, enemies)) {
      if (forceIdx == enemyIdx) continue;

      if (check_collision(forceTransform, forceCollider, enemyTransform,
                          enemyCollider)) {
        enemy.current -= force.contactDamage;
        std::cout << "Force hit Enemy " << enemyIdx << "! HP: " << enemy.current
                  << std::endl;

        if (enemy.current <= 0) {
          registry.kill_entity(Entity(enemyIdx));
        }
      }
    }

    for (auto&& [bossIdx, bossTransform, bossCollider, boss] :
         IndexedZipper(transforms, colliders, bosses)) {
      if (forceIdx == bossIdx) continue;

      if (check_collision(forceTransform, forceCollider, bossTransform,
                          bossCollider)) {
        boss.current -= force.contactDamage;
        std::cout << "Force hit Boss " << bossIdx << "! HP: " << boss.current
                  << std::endl;

        if (boss.current <= 0) {
          registry.kill_entity(Entity(bossIdx));
        }
      }
    }

    for (auto&& [partIdx, partTransform, partCollider, part] :
         IndexedZipper(transforms, colliders, bossParts)) {
      if (forceIdx == partIdx) continue;
      if (!part.alive) continue;

      if (check_collision(forceTransform, forceCollider, partTransform,
                          partCollider)) {
        part.hp -= force.contactDamage;
        std::cout << "Force hit BossPart " << partIdx << "! HP: " << part.hp
                  << std::endl;

        if (part.hp <= 0) {
          part.alive = false;
          registry.kill_entity(Entity(partIdx));
        }
      }
    }

    if (force.blocksProjectiles) {
      for (auto&& [projIdx, projTransform, projCollider, proj] :
           IndexedZipper(transforms, colliders, projectiles)) {
        if (forceIdx == projIdx) continue;
        if (!proj.isActive) continue;

        size_t ownerId = proj.ownerId;
        bool isEnemyProjectile =
            (ownerId >= registry.get_components<PlayerEntity>().size() ||
             !registry.get_components<PlayerEntity>()[ownerId].has_value());

        if (!isEnemyProjectile) continue;

        if (check_collision(forceTransform, forceCollider, projTransform,
                            projCollider)) {
          proj.isActive = false;
          registry.kill_entity(Entity(projIdx));
          std::cout << "Force BLOCKED enemy projectile " << projIdx << "!"
                    << std::endl;
        }
      }
    }
  }
}
