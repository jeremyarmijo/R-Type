// Copyright 2025 Dalia Guiz
#pragma once
#include "../Physics2D.hpp"

struct ProjectTile {
  float speed;
  Vector2 direction;
  float timer;
  bool is_existing;
  Uint32 damage;
  bool is_player_projectTile;
  bool active;

  ProjectTile(float s = 0.f, Vector2 dir = {0.f, 0.f}, float t = 0.f,
              bool existing = true, Uint32 dmg = 0, bool fromPlayer = true,
              bool act = true)
      : speed(s),
        direction(dir),
        timer(t),
        is_existing(existing),
        damage(dmg),
        is_player_projectTile(fromPlayer),
        active(act) {}
};
