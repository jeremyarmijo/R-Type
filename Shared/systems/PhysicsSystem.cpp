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

void collision_detection_system(Registry& registry,
                                const SparseArray<Transform>& transforms,
                                const SparseArray<BoxCollider>& colliders,
                                SparseArray<RigidBody>& rigidbodies) {
  std::vector<size_t> collidable_entities;
  for (auto&& [idx, transform, collider] :
       IndexedZipper(transforms, colliders)) {
    collidable_entities.push_back(idx);
  }

  for (size_t i = 0; i < collidable_entities.size(); ++i) {
    for (size_t j = i + 1; j < collidable_entities.size(); ++j) {
      size_t entityA = collidable_entities[i];
      size_t entityB = collidable_entities[j];

      const auto& transformA = transforms[entityA];
      const auto& colliderA = colliders[entityA];
      const auto& transformB = transforms[entityB];
      const auto& colliderB = colliders[entityB];

      if (!transformA || !colliderA || !transformB || !colliderB) {
        continue;
      }

      if (check_collision(*transformA, *colliderA, *transformB, *colliderB)) {
        auto& rigidbodyA = rigidbodies[entityA];
        auto& rigidbodyB = rigidbodies[entityB];

        if (rigidbodyA && rigidbodyB) {
          resolve_collision(*rigidbodyA, *rigidbodyB, *transformA, *transformB);
        }
      }
    }
  }
}

bool check_collision(const Transform& transformA, const BoxCollider& colliderA,
                     const Transform& transformB,
                     const BoxCollider& colliderB) {
  auto boundsA = colliderA.GetBounds(transformA.position);
  auto boundsB = colliderB.GetBounds(transformB.position);

  bool collisionX =
      boundsA.right >= boundsB.left && boundsB.right >= boundsA.left;
  bool collisionY =
      boundsA.bottom >= boundsB.top && boundsB.bottom >= boundsA.top;

  return collisionX && collisionY;
}

void resolve_collision(RigidBody& rigidbodyA, RigidBody& rigidbodyB,
                       const Transform& transformA,
                       const Transform& transformB) {
  Vector2 relativeVelocity = rigidbodyA.velocity - rigidbodyB.velocity;
  float restitution = std::min(rigidbodyA.restitution, rigidbodyB.restitution);

  if (!rigidbodyA.isStatic && !rigidbodyB.isStatic) {
    float totalMass = rigidbodyA.mass + rigidbodyB.mass;
    float ratioA = rigidbodyB.mass / totalMass;
    float ratioB = rigidbodyA.mass / totalMass;

    Vector2 impulse = relativeVelocity * restitution;
    rigidbodyA.velocity = rigidbodyA.velocity - impulse * ratioA;
    rigidbodyB.velocity = rigidbodyB.velocity + impulse * ratioB;

  } else if (!rigidbodyA.isStatic) {
    // bounce off B
    rigidbodyA.velocity = rigidbodyA.velocity * -restitution;

  } else if (!rigidbodyB.isStatic) {
    // bounce off A
    rigidbodyB.velocity = rigidbodyB.velocity * -restitution;
  }

  // prevent endless bouncing
  const float dampingFactor = 0.95f;
  if (!rigidbodyA.isStatic) {
    rigidbodyA.velocity = rigidbodyA.velocity * dampingFactor;
  }
  if (!rigidbodyB.isStatic) {
    rigidbodyB.velocity = rigidbodyB.velocity * dampingFactor;
  }
}

void physics_system(Registry& registry, float deltaTime,
                    const Vector2& gravity) {
  auto& transforms = registry.get_components<Transform>();
  auto& rigidbodies = registry.get_components<RigidBody>();
  auto& colliders = registry.get_components<BoxCollider>();

  physics_movement_system(registry, transforms, rigidbodies, deltaTime,
                          gravity);
  collision_detection_system(registry, transforms, colliders, rigidbodies);
}
