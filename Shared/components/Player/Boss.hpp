// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_PLAYER_BOSS_HPP_
#define SHARED_COMPONENTS_PLAYER_BOSS_HPP_

#include "../Physics2D.hpp"
enum class BossType {
  BigShip,
  Snake,       // Boss serpent multi-segments
  BydoEye,     // Boss statique qui tire beaucoup
  Battleship,  // Boss qui spawn des ennemis
  FinalBoss    // Ultime boss
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

  Boss(BossType t, BossPhase p = BossPhase::Phase1, float spd = 100.f,
       Vector2 dir = {0.f, 0.f}, float amp = 40.f, int cur = 500)
      : type(t),
        phase(p),
        direction(dir),
        timer(0.f),
        speed(spd),
        amplitude(amp),
        current(cur) {}
};

#endif  // SHARED_COMPONENTS_PLAYER_BOSS_HPP_
