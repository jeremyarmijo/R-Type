#pragma once
#include "Player/PlayerEntity.hpp"
#include "physics/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void charged_shoot_system(Registry& registry, float deltaTime);
