// Copyright 2025 Dalia Guiz
#pragma once
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void player_movement_system(SparseArray<Transform>& transforms,
    SparseArray<RigidBody>& rigidbodies,
    SparseArray<PlayerControlled>& players,
    float deltaTime);

void enemy_movement_system(SparseArray<Transform>& transforms,
    SparseArray<RigidBody>& rigidbodies,
    SparseArray<Enemy>& enemies,
    float deltaTime);

void projectTile_movement_system(SparseArray<Transform>& transforms,
    SparseArray<RigidBody>& rigidbodies,
    SparseArray<ProjectTile>& projectiles,
    Registry& registry,
    float deltaTime);

void boss_movement_system(SparseArray<Transform>& transforms,
    SparseArray<RigidBody>& rigidbodies,
    SparseArray<Boss>& bosses,
    float deltaTime);
