#pragma once
#include "ecs/Registry.hpp"

struct PlayerController {
  Vector2 get_input_direction() const;
};
