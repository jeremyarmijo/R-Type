#pragma once
#include "Player/PlayerEntity.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "inputs/InputSystem.hpp"

void charged_shoot_system(Registry& registry, float deltaTime);
