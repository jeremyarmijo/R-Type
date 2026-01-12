#pragma once
#include "components/Physics2D.hpp"
#include "components/Player/Projectile.hpp"
#include "ecs/Registry.hpp"

void projectile_lifetime_system(Registry& registry,
                                SparseArray<Projectile>& projectiles,
                                float deltaTime);

void projectile_collision_system(Registry& registry,
                                 const SparseArray<Transform>& transforms,
                                 const SparseArray<BoxCollider>& colliders,
                                 SparseArray<Projectile>& projectiles);

Entity spawn_projectile(Registry& registry, Vector2 position, Vector2 direction,
                        float speed, size_t ownerId);

Entity spawn_player_projectile(Registry& registry, Vector2 position,
                               Vector2 direction, float speed, size_t ownerId);
