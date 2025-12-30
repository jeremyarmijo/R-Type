#pragma once
#include <vector>
#include "components/Levels.hpp"
#include "ecs/Registry.hpp"

void update_level_system(Registry& registry,
                         SparseArray<LevelComponent>& levels,
                         SparseArray<Enemy>& enemies, float deltaTime);

std::vector<LevelComponent> createLevels();
