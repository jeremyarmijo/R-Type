#pragma once

#include "components/TileMap.hpp"
#include "ecs/Registry.hpp"

void tilemap_collision_system(Registry& registry);
void tilemap_enemy_collision_system(Registry& registry);
