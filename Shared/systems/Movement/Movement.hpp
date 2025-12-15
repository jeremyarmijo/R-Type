// Copyright 2025 Dalia Guiz
#pragma once
#include "components/Physics2D.hpp"
#include "components/Player/Boss.hpp"
#include "components/Player/Enemy.hpp"
#include "components/Player/PlayerEntity.hpp"
#include "components/Player/Projectile.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void enemy_movement_system(SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Enemy>& enemies, float deltaTime);

void Projectile_movement_system(SparseArray<Transform>& transforms,
                                SparseArray<RigidBody>& rigidbodies,
                                SparseArray<Projectile>& projectiles,
                                Registry& registry, float deltaTime);

void boss_movement_system(SparseArray<Transform>& transforms,
                          SparseArray<RigidBody>& rigidbodies,
                          SparseArray<Boss>& bosses, float deltaTime);
