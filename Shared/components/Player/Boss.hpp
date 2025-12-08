// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_PLAYER_BOSS_HPP_
    #define SHARED_COMPONENTS_PLAYER_BOSS_HPP_

#include "../Physics2D.hpp"
enum class BossType {
    BigShip,
    Snake,          // Boss serpent multi-segments
    BydoEye ,           // Boss statique qui tire beaucoup
    Battleship,       // Boss qui spawn des ennemis
    FinalBoss      // Ultime boss
};

enum class BossPhase {
    Phase1,
    Phase2,
    Phase3
};

struct Boss {
    BossType type;
    BossPhase phase;
    Vector2 direction;
    float timer = 0.f;
    float speed;
    float amplitude = 40.f;
};

#endif  // SHARED_COMPONENTS_PLAYER_BOSS_HPP_
