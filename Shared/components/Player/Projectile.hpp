// Copyright 2025 Ziad
#pragma once
#include "../Physics2D.hpp"

struct ProjectTile {
  float damage;
  float speed;
  Vector2 direction;

  float lifetime;
  float currentLife;
  size_t ownerId;
  bool is_active = true;

  explicit ProjectTile(float dmg = 10.0f, float spd = 300.0f, Vector2 dir = {1.0f, 0.0f}, float life = 5.0f, size_t owner = 0)
      : damage(dmg), speed(spd), direction(dir), lifetime(life), currentLife(0.0f), ownerId(owner), is_active(true) {}
};
