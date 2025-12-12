#include "../systems/PhysicsSystem.hpp"

#include <algorithm>
#include <vector>

void physics_movement_system(Registry& registry,
                             SparseArray<Transform>& transforms,
                             SparseArray<RigidBody>& rigidbodies,
                             float deltaTime, const Vector2& gravity) {
  for (auto&& [transform, rigidbody] : Zipper(transforms, rigidbodies)) {
    if (rigidbody.isStatic) {
      continue;
    }

    rigidbody.acceleration += gravity;
    rigidbody.velocity += rigidbody.acceleration * deltaTime;
    transform.position += rigidbody.velocity * deltaTime;
    rigidbody.acceleration = {0, 0};
  }
}
