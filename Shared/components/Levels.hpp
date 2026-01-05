// components/Levels.hpp
#pragma once
#include <optional>
#include <vector>

#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "components/Physics2D.hpp"

struct Wave {
  std::vector<EnemyType> enemyTypes;
  std::vector<int> enemiesPerType;
  std::vector<Vector2> spawnPositions;
  bool isBossWave = false;
  std::optional<BossType> bossType;
  int bossHP = 0;
};

struct LevelComponent {
  std::vector<Wave> waves;
  int currentWave = -1;
  float waveDelayTimer = 0.0f;
  bool finishedLevel = false;
  bool initialized = false;
  int levelIndex = 0;

  LevelComponent() = default;
  explicit LevelComponent(const std::vector<Wave>& ws)
      : waves(ws), currentWave(-1) {}
};
