// Copyright 2025 Dalia Guiz
#pragma once
#include "../Physics2D.hpp"

enum class EnemyType { Basic, Zigzag, Wave };

struct Enemy {
  EnemyType type;
  float speed;
  Vector2 direction;
  float amplitude;
  float timer;
  int scoreValue;
  int weaponId;
  int current;

  Enemy(EnemyType t, float spd, Vector2 dir = {0.f, 0.f}, float amp = 20.f,
        int cur = 200, int score = 0, int weapon = 0)
      : type(t), speed(spd), direction(dir), amplitude(amp), timer(0.f),
        scoreValue(score), weaponId(weapon), current(cur) {}
};
