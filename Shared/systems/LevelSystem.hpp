#pragma once
#include <vector>

#include "components/Levels.hpp"
#include "ecs/Registry.hpp"

bool update_level_system(Registry& registry, float deltaTime, int levelIndex, uint8_t diff = 1);
std::vector<LevelComponent> createLevels();
