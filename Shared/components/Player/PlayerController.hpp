// Copyright 2025 Dalia Guiz
#pragma once
#include "components/Physics2D.hpp"

struct PlayerControlled {
  int player_id;
  Vector2 input;
  float speed;
};

struct Health {
  int current;
  int max;
  bool isAlive = true;
  float invtimer;
};

struct Weapon {
  int weaponId;
  bool hasForce;
};

struct PlayerId {
  int playerId;
  int team;
  int score;
};

struct Death {
  Entity deadEntity;
  Entity killer;
  CollisionCategory type;
  Vector2 position;
  Death(Entity deadEntity, Entity killer, CollisionCategory type,
        Vector2 position)
      : deadEntity(deadEntity),
        killer(killer),
        type(type),
        position(position) {}
};
