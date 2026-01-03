#pragma once
#include <vector>

#include "components/Levels.hpp"
#include "ecs/Registry.hpp"

bool update_level_system(Registry& registry, float deltaTime, int levelIndex);
std::vector<LevelComponent> createLevels();
