// Copyright 2025 Dalia Guiz
#pragma once
#include "../../components/Collision/CollisionController.hpp"
#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void gamePlay_Collision_system(
    Registry& registry, SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders, SparseArray<PlayerEntity>& players,
    SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
    SparseArray<Items>& items, SparseArray<Projectile>& projectiles);

CollisionCategory get_entity_category(size_t entityId, Registry& registry);

void apply_damage(Registry& registry, const Collision& collision,
                  SparseArray<Transform>& transforms,
                  SparseArray<PlayerEntity>& players,
                  SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
                  SparseArray<Projectile>& projectiles);

inline bool check_collision(const Transform& t1, const BoxCollider& c1,
                            const Transform& t2, const BoxCollider& c2) {
  return t1.position.x < t2.position.x + c2.width &&
         t1.position.x + c1.width > t2.position.x &&
         t1.position.y < t2.position.y + c2.height &&
         t1.position.y + c1.height > t2.position.y;
}
