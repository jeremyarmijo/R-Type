#include "systems/BoundsSystem.hpp"

#include <iostream>
#include <vector>

#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"

void bounds_check_system(Registry& registry, SparseArray<Transform>& transforms,
                         SparseArray<BoxCollider>& colliders,
                         SparseArray<RigidBody>& rigidbodies) {
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();

  for (auto&& [entityId, transform, collider, player] :
       IndexedZipper(transforms, colliders, players)) {
    float halfWidth = collider.width / 2.0f;
    float halfHeight = collider.height / 2.0f;

    if (transform.position.x - halfWidth < 0.0f) {
      transform.position.x = halfWidth;
      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.x = 0.0f;
      }
    } else if (transform.position.x + halfWidth > 800) {
      transform.position.x = 800 - halfWidth;
      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.x = 0.0f;
      }
    }

    if (transform.position.y - halfHeight < 0.0f) {
      transform.position.y = halfHeight;
      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.y = 0.0f;
      }
    } else if (transform.position.y + halfHeight > 600) {
      transform.position.y = 600 - halfHeight;
      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.y = 0.0f;
      }
    }
  }

  std::vector<Entity> toKill;

  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i].has_value() && i < transforms.size() &&
        transforms[i].has_value()) {
      float x = transforms[i]->position.x;
      float y = transforms[i]->position.y;

      if (x < -100.0f || y < -100.0f || y > 700.0f) {
        toKill.push_back(registry.entity_from_index(i));
      }
    }
  }

  for (size_t i = 0; i < bosses.size(); ++i) {
    if (bosses[i].has_value() && i < transforms.size() &&
        transforms[i].has_value()) {
      float x = transforms[i]->position.x;
      float y = transforms[i]->position.y;

      if (x < -200.0f || y < -200.0f || y > 800.0f) {
        std::cout << "[BoundsSystem] Boss at index " << i
                  << " went out of bounds (x=" << x << "), killing it"
                  << std::endl;
        toKill.push_back(registry.entity_from_index(i));
      }
    }
  }

  for (Entity e : toKill) {
    registry.kill_entity(e);
  }
}
