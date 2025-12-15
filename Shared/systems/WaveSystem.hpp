#ifndef WAVE_SYSTEM_HPP
#define WAVE_SYSTEM_HPP
#pragma once
#include "Player/Enemy.hpp"
#include "ecs/Registry.hpp"

void weapon_cooldown_system(Registry& registry, SparseArray<Enemy>& weapons,
                            float deltaTime);

#endif  // WEAPON_SYSTEM_HPP
