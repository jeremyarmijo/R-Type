#pragma once
#include "ecs/Registry.hpp"

void weapon_cooldown_system(Registry& registry,
                            SparseArray<Weapon>& weapons,
                            float deltaTime);

void weapon_firing_system(Registry& registry,
                          SparseArray<Weapon>& weapons,
                          SparseArray<Transform>& transforms,
                          const std::function<bool(size_t)>& is_firing_input,
                          float deltaTime);

void weapon_reload_system(Registry& registry,
                           SparseArray<Weapon>& weapons,
                           float deltaTime);

bool can_fire(const Weapon& weapon);
void consume_ammo(Weapon& weapon);
bool needs_reload(const Weapon& weapon);
void start_reload(Weapon& weapon);