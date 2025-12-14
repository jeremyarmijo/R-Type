// Copyright 2025 Dalia Guiz
#pragma once
#include "Collision/CollisionCotroller.hpp"
#include "components/Physics2D.hpp"

struct PlayerControlled {
  int player_id = 0;
  Vector2 input{};
  float speed = 100.f;
  int current = 100;
  int max = 100;
  bool isAlive = true;
  float invtimer = 0.f;
  int weaponId = 0;
  bool hasForce = false;
  int score = 0;
};
