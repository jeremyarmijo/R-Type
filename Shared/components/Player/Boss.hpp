// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_PLAYER_BOSS_HPP_
#define SHARED_COMPONENTS_PLAYER_BOSS_HPP_

#include "../Physics2D.hpp"
#include "ecs/Entity.hpp"

enum class BossType {
  FinalBoss,   // Ultime boss
  Gomander_snake,       // Boss serpent multi-segments
  BigShip,
  BydoEye,     // Boss statique qui tire beaucoup
  Bydo_Battleship,  // Boss qui spawn des ennemis
};

enum class BossPhase { Phase1, Phase2, Phase3 };

struct Boss {
  BossType type;
  BossPhase phase;
  Vector2 direction;
  float timer;
  float speed;
  float amplitude;
  int current;
  int hp;
  int contact_damage;
  int segmentCount;
  uint32_t scoreValue;

  Boss(BossType t, BossPhase p = BossPhase::Phase1, float spd = 100.f,
       Vector2 dir = {0.f, 0.f}, float amp = 40.f, int cur = 500, int dmg = 10,
       int segment = 0)
      : type(t),
        phase(p),
        direction(dir),
        timer(0.f),
        speed(spd),
        amplitude(amp),
        current(cur),
        contact_damage(dmg),
        segmentCount(segment) {}
};


#endif  // SHARED_COMPONENTS_PLAYER_BOSS_HPP_
