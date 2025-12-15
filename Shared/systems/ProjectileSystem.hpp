#pragma once
#include "ecs/Registry.hpp"
#include "components/Player/Projectile.hpp"
#include "components/Physics2D.hpp"

void projectile_lifetime_system(Registry& registry,
                                SparseArray<Projectile>& projectiles,
                                float deltaTime);

void projectile_collision_system(Registry& registry,
                                 const SparseArray<Transform>& transforms,
                                 const SparseArray<BoxCollider>& colliders,
                                 SparseArray<Projectile>& projectiles);

Entity spawn_projectile(Registry& registry,
                       Vector2 position,
                       Vector2 direction,
                       float speed,
                       size_t ownerId);
