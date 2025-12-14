#pragma once
#include <vector>
#include <utility>

#include "./Enemy.hpp"

struct EnemySpawning {
  std::vector<Vector2> spawnPoints;
  float spawnInterval;
  float timer;
  int maxEnemiesAct;
  EnemyType type;
  int spawnAmount;
  bool waveActive;

  EnemySpawning(std::vector<Vector2> points = {}, float interval = 1.0f,
                int maxEnemies = 5, EnemyType t = EnemyType::Basic,
                int amount = 1, bool active = false)
      : spawnPoints(std::move(points)),
        spawnInterval(interval),
        timer(0.0f),
        maxEnemiesAct(maxEnemies),
        type(t),
        spawnAmount(amount),
        waveActive(active) {}
};
