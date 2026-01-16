// Copyright 2025 Dalia Guiz
#pragma once
#include "physics/Physics2D.hpp"

enum class EnemyType { Basic, Zigzag, Chase };

struct Enemy {
  EnemyType type;
  float speed;
  Vector2 direction;
  float amplitude;
  float timer;
  float timeSinceLastShot;
  int scoreValue;
  int weaponId;
  int current;
  int contact_damage;

  Enemy(EnemyType t, float spd, Vector2 dir = {0.f, 0.f}, float amp = 80.f,
        int cur = 50, int score = 0, int weapon = 0, int dmg = 10)
      : type(t),
        speed(spd),
        direction(dir),
        amplitude(amp),
        timer(0.f),
        timeSinceLastShot(0.f),
        scoreValue(score),
        weaponId(weapon),
        current(cur),
        contact_damage(dmg) {}
};
