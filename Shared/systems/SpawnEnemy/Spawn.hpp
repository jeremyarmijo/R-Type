// Copyright 2025 Dalia Guiz
#ifndef SHARED_SYSTEMS_SPAWNENEMY_SPAWN_HPP_
#define SHARED_SYSTEMS_SPAWNENEMY_SPAWN_HPP_

#include "Player/EnemySpawn.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void enemy_spawner_system(Registry& registry,
                          SparseArray<EnemySpawning>& spawners,
                          float deltaTime);

#endif  // SHARED_SYSTEMS_SPAWNENEMY_SPAWN_HPP_
