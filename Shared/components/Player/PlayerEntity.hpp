// Copyright 2025 Dalia Guiz
#pragma once
#include <algorithm>
#include "Collision/CollisionController.hpp"
#include "components/Physics2D.hpp"

struct PlayerEntity {
  int player_id;
  Vector2 input;
  float speed;
  int current;
  int max;
  bool isAlive;
  float invtimer;
  Weapon weapon;
  bool hasForce;
  int score;

  PlayerEntity(int id = 0, float s = 100.f, int cur = 100, int mx = 100,
                   bool alive = true, float inv = 0.f, Weapon weapon = {},
                   bool force = false, int sc = 0)

      : player_id(id),
        speed(s),
        current(cur),
        max(mx),
        isAlive(alive),
        invtimer(inv),
        weapon(weapon),
        hasForce(force),
        score(sc) {}
};

struct Weapon {
  int weaponId;
  bool hasForce;

  explicit Weapon(int id = 0, bool force = false)
      : weaponId(id), hasForce(force) {}
};
