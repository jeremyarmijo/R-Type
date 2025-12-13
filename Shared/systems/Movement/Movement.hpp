// Copyright 2025 Dalia Guiz
#pragma once
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/ProjectTile.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "inputs/InputManager.hpp"

void enemy_movement_system(SparseArray<Transform>& transforms,
                           SparseArray<RigidBody>& rigidbodies,
                           SparseArray<Enemy>& enemies, float deltaTime);

void boss_movement_system(SparseArray<Transform>& transforms,
                          SparseArray<RigidBody>& rigidbodies,
                          SparseArray<Boss>& bosses, float deltaTime);
