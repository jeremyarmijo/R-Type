// Copyright 2025 Dalia Guiz
#pragma once
#include <algorithm>

#include "Collision/CollisionController.hpp"
#include "Player/Weapon.hpp"
#include "components/Physics2D.hpp"

struct PlayerShoot {
  bool isCharging = false;
  float chargeTime = 0.0f;
  float maxChargeTime = 2.0f;

  int GetChargeLevel() const {
    if (chargeTime >= 1.5f) return 3;  // Gros tir
    if (chargeTime >= 1.0f) return 2;  // Moyen
    if (chargeTime >= 0.5f) return 1;  // Petit chargé
    return 0;  // Normal
  }

  void Reset() {
    isCharging = false;
    chargeTime = 0.0f;
  }
};

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
  PlayerShoot shoot;

  PlayerEntity(int id = 0, float s = 150.f, int cur = 100, int mx = 100,
               bool alive = true, float inv = 0.f, Weapon weapon = Weapon(),
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
