// Copyright 2025 Dalia Guiz
#pragma once
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void player_movement_system(Registry& registry, float deltaTime);
void ennemy_movement_system(Registry& registry, float deltaTime);
void projectTile_movement_system(Registry& registry, float deltaTime);
void boss_movement_system(Registry& registry, float deltaTime);
