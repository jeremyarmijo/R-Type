// #pragma once

#include "systems/WaveSystem.hpp"

#include <iostream>
#include <random>

#include "Helpers/EntityHelper.hpp"
#include "SpawnEnemy/Spawn.hpp"
#include "ecs/Zipper.hpp"

Vector2 get_random_pos() {
  Vector2 pos;

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> disY(50, 500);

  pos.x = 750;
  pos.y = static_cast<float>(disY(gen));
  return pos;
}
Vector2 get_boss_spawn_pos() { return {700.0f, 300.0f}; }

bool checkWaveEnd(Registry& registry, SparseArray<Enemy>& enemies) {
  int active_enemies = 0;
  for (const auto& enemy : enemies) {
    if (enemy.has_value()) {
      active_enemies++;
    }
  }
  return active_enemies == 0;
}

void create_multiples_enemies(Registry& registry, EnemyType type,
                              int nbEnemies) {
  for (int i = 0; i < nbEnemies; i++) {
    Vector2 pos = get_random_pos();
    createEnemy(registry, type, pos);
  }
}

void enemy_wave_system(Registry& registry, SparseArray<Enemy>& enemies,
                       float deltaTime, int nbWave, int difficulty) {
  static bool waveOn = false;
  static int currentWave = 0;
  static int level = 0;
  static float waveDelayTimer = 3.0f;
  const float TIME_BETWEEN_WAVES = 3.0f;
  static bool bossSpawned = false;

  if (waveOn) {
    if (checkWaveEnd(registry, enemies)) {
      std::cout << "Wave " << currentWave << " ended. Starting delay."
                << std::endl;
      if (currentWave == 4 && !bossSpawned) {
        std::cout << "BOSS WAVE TRIGGERED!" << std::endl;
        Vector2 bossPos = get_boss_spawn_pos();
        createBoss(registry, BossType::FinalBoss, bossPos, BossPhase::Phase1,
                   500);

        bossSpawned = true;
        waveOn = true;
        return;
      }
      waveOn = false;
      waveDelayTimer = TIME_BETWEEN_WAVES;
    }
    return;
  }

  if (bossSpawned) {
    if (checkWaveEnd(registry, enemies)) {
      std::cout << "Boss defeated! Resuming waves." << std::endl;
      bossSpawned = false;
      currentWave = 0;
      level += 1;
      waveDelayTimer = TIME_BETWEEN_WAVES;
    }
    return;
  }

  if (waveDelayTimer > 0.0f) {
    waveDelayTimer -= deltaTime;
    std::cout << "Wave delay: " << waveDelayTimer << "s remaining."
              << std::endl;

    if (waveDelayTimer > 0.0f) {
      return;
    }
  }

  if (!waveOn && !bossSpawned) {
    std::cout << "Spawning Wave " << currentWave + 1 << "!" << std::endl;

    int nbBasic = ((2 + currentWave) / 2) * (difficulty + level);
    int nbZigzag = ((2 + currentWave) / 2) * (difficulty + level);
    int nbChaseEnemies = ((1 + currentWave) / 2) * (difficulty + level);

    create_multiples_enemies(registry, EnemyType::Basic, nbBasic);
    create_multiples_enemies(registry, EnemyType::Zigzag, nbZigzag);
    create_multiples_enemies(registry, EnemyType::Chase, nbChaseEnemies);

    currentWave++;
    waveOn = true;
    waveDelayTimer = 0.0f;
  }
}
