#include "systems/BoundsSystem.hpp"
#include "Player/PlayerEntity.hpp"

void bounds_check_system(Registry& registry, SparseArray<Transform>& transforms,
                         SparseArray<BoxCollider>& colliders,
                         SparseArray<RigidBody>& rigidbodies) {
  auto& players = registry.get_components<PlayerEntity>();
  for (auto&& [entityId, transform, collider, player] :
       IndexedZipper(transforms, colliders, players)) {
    float halfWidth = collider.width / 2.0f;
    float halfHeight = collider.height / 2.0f;

    if (transform.position.x - halfWidth < 0.0f) {
      transform.position.x = halfWidth;

      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.x = 0.0f;
      }
    }

    else if (transform.position.x + halfWidth > 800) {
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
    }

    else if (transform.position.y + halfHeight > 600) {
      transform.position.y = 600 - halfHeight;

      if (entityId < rigidbodies.size() && rigidbodies[entityId].has_value()) {
        rigidbodies[entityId]->velocity.y = 0.0f;
      }
    }
  }
}
