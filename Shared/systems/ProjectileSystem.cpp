#include "systems/ProjectileSystem.hpp"

#include <utility>

#include "systems/PhysicsSystem.hpp"
#include "ecs/Zipper.hpp"

void projectile_lifetime_system(Registry& registry,
                                SparseArray<Projectile>& projectiles,
                                float deltaTime) {
    for (auto&& [idx, projectile] : IndexedZipper(projectiles)) {
        if (!projectile.isActive) continue;

        projectile.currentLife += deltaTime;

        if (projectile.currentLife >= projectile.lifetime) {
            registry.kill_entity(Entity(idx));
        }
    }
}

void projectile_collision_system(Registry& registry,
                                 const SparseArray<Transform>& transforms,
                                 const SparseArray<BoxCollider>& colliders,
                                 SparseArray<Projectile>& projectiles) {
    for (auto&& [projIdx, projectile, projTransform, projCollider] :
         IndexedZipper(projectiles, transforms, colliders)) {
        if (!projectile.isActive) continue;

        for (auto&& [targetIdx, targetTransform, targetCollider] :
             IndexedZipper(transforms, colliders)) {
            if (projIdx == targetIdx) continue;

            if (check_collision(projTransform, projCollider,
                              targetTransform, targetCollider)) {
                projectile.isActive = false;
                registry.kill_entity(Entity(projIdx));

                // TODO(ZiadBengherabi) apply damage to target entity
                break;
            }
        }
    }
}

Entity spawn_projectile(Registry& registry,
                       Vector2 position,
                       Vector2 direction,
                       float speed,
                       size_t ownerId) {
    Entity projectile = registry.spawn_entity();

    // Transform
    registry.emplace_component<Transform>(projectile, position);

    // Physics
    Vector2 velocity = direction.Normalized() * speed;
    RigidBody rb(0.1f, 0.0f, false);
    rb.velocity = velocity;
    registry.emplace_component<RigidBody>(projectile, std::move(rb));

    // Collider
    registry.emplace_component<BoxCollider>(projectile, 8.0f, 8.0f);

    // Projectile component
    registry.emplace_component<Projectile>(projectile, 10.0f, 3.0f, ownerId);

    // Sprite
// registry.emplace_component<Sprite>(projectile, "", SDL_Rect{0, 0, 8, 8});

    return projectile;
}
