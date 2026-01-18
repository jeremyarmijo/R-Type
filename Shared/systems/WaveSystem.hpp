#ifndef WAVE_SYSTEM_HPP
#define WAVE_SYSTEM_HPP
#pragma once
#include "Player/Enemy.hpp"
#include "ecs/Registry.hpp"

void enemy_wave_system(Registry& registry, SparseArray<Enemy>& weapons,
                       float deltaTime, int nbWave, int diff);

#endif  // WEAPON_SYSTEM_HPP
