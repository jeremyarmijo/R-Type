// Copyright 2025 Dalia Guiz
#pragma once
#include "../Physics2D.hpp"

struct ProjectTile {
  float speed;
  Vector2 direction;
  float timer = 0.f;
  bool is_existing;
  Uint32 damage;
  bool is_player_projectTile;
  bool active = true;
};  // cpplint fix
