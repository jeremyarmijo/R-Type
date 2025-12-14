// Copyright 2025 Dalia Guiz
#pragma once
#include "../Physics2D.hpp"

enum class EnemyType {
    Basic,
    Zigzag,
    Wave
};


struct Enemy {
    EnemyType type;
    float speed;
    Vector2 direction;
    float amplitude = 20.f;
    float timer = 0.f;
    int scoreValue;
    int weaponId;
    int current = 200;
};

