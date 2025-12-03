#pragma once
#include "ecs/Registry.hpp"
//#include "components/Transform.hpp"
#include "components/Physics2D.hpp"
#include "../Player/PlayerController.hpp"
#include "ecs/Zipper.hpp"

void player_movement_system(Registry& registry, float deltaTime, float speed);
