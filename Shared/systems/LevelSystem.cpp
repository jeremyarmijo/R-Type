#include <vector>
#include <iostream>
#include "systems/LevelSystem.hpp"
#include "Helpers/EntityHelper.hpp"
#include "components/Levels.hpp"
#include "components/Player/Enemy.hpp"
#include "ecs/Zipper.hpp"

const float TIME_BETWEEN_WAVES = 3.0f;

std::vector<LevelComponent> createLevels() {
  std::vector<LevelComponent> levels;
  // level #1
  levels.emplace_back(std::vector<Wave>{
      Wave{{EnemyType::Basic},
           {3},
           {{700, 100}, {700, 200}, {700, 300}},
           false,
           std::nullopt,
           0},
      Wave{{EnemyType::Basic, EnemyType::Zigzag},
           {2, 2},
           {{700, 120}, {700, 220}, {700, 320}, {700, 420}},
           false,
           std::nullopt,
           0},
      Wave{{}, {0}, {{700, 250}}, true, BossType::BigShip, 300}});
  // level #2
  levels.emplace_back(std::vector<Wave>{
      Wave{{EnemyType::Zigzag},
           {4},
           {{700, 80}, {700, 140}, {700, 200}, {700, 260}},
           false,
           std::nullopt,
           0},
      Wave{{EnemyType::Zigzag, EnemyType::Chase},
           {3, 2},
           {{700, 110}, {700, 170}, {700, 230}, {700, 290}, {700, 350}},
           false,
           std::nullopt,
           0},
      Wave{{}, {0}, {{700, 300}}, true, BossType::Snake, 500}});
  // level #3
  levels.emplace_back(std::vector<Wave>{
      Wave{{EnemyType::Chase},
           {5},
           {{700, 60}, {700, 120}, {700, 180}, {700, 240}, {700, 300}},
           false,
           std::nullopt,
           0},
      Wave{{EnemyType::Zigzag, EnemyType::Chase},
           {4, 4},
           {{700, 90},
            {700, 150},
            {700, 210},
            {700, 270},
            {700, 330},
            {700, 390},
            {700, 450},
            {700, 510}},
           false,
           std::nullopt,
           0},
      Wave{{}, {0}, {{700, 300}}, true, BossType::FinalBoss, 1200}});
  return levels;
}

void update_level_system(Registry& registry,
                         SparseArray<LevelComponent>& levels,
                         SparseArray<Enemy>& enemies, float deltaTime) {
  for (auto&& [levelIndex, level] : IndexedZipper(levels)) {
    if (level.finishedLevel) continue;
    if (!level.initialized) {
      std::cout << "Level " << levelIndex
                << " currentWave = " << level.currentWave << std::endl;
      level.initialized = true;  // on print une seule fois
    }
    bool anyEnemyAlive = false;
    for (auto&& en : enemies) {
      if (en.has_value()) {
        anyEnemyAlive = true;
        break;
      }
    }

    if (!anyEnemyAlive) {
      level.waveDelayTimer += deltaTime;

      if (level.waveDelayTimer < TIME_BETWEEN_WAVES) {
        continue;
      }

      level.waveDelayTimer = 0.0f;
      level.currentWave++;

      if (level.currentWave >= static_cast<int>(level.waves.size())) {
        level.finishedLevel = true;
        continue;
      }
      Wave& wave = level.waves[level.currentWave];

      float speedMultiplier = 1.0f + 0.15f * static_cast<float>(levelIndex);

      if (wave.isBossWave) {
        BossType btype =
            wave.bossType.value_or((levelIndex == 0)   ? BossType::BigShip
                                   : (levelIndex == 1) ? BossType::Snake
                                                       : BossType::FinalBoss);

        Vector2 bossPos = wave.spawnPositions.empty() ? Vector2{700.0f, 300.0f}
                                                      : wave.spawnPositions[0];
        int bossHP = (wave.bossHP > 0) ? wave.bossHP : 500 + levelIndex * 200;

        Entity bossEntity =
            createBoss(registry, btype, bossPos, BossPhase::Phase1, bossHP);

        auto& bossArr = registry.get_components<Boss>();
        size_t id = static_cast<size_t>(bossEntity);
        if (id < bossArr.size() && bossArr[id].has_value()) {
          bossArr[id]->speed *= speedMultiplier;
        }

      } else {
        for (size_t t = 0; t < wave.enemyTypes.size(); ++t) {
          EnemyType type = wave.enemyTypes[t];
          int count =
              (t < wave.enemiesPerType.size()) ? wave.enemiesPerType[t] : 1;

          for (int k = 0; k < count; ++k) {
            Vector2 spawnPos =
                wave.spawnPositions.empty()
                    ? Vector2{750.0f, static_cast<float>(50 + (k * 30) % 500)}
                    : wave.spawnPositions[(k) % wave.spawnPositions.size()];

            Entity e = createEnemy(registry, type, spawnPos);
            auto& enemiesArr = registry.get_components<Enemy>();
            size_t eid = static_cast<size_t>(e);
            if (eid < enemiesArr.size() && enemiesArr[eid].has_value()) {
              enemiesArr[eid]->speed *= speedMultiplier;
              enemiesArr[eid]->current = static_cast<int>(
                  enemiesArr[eid]->current * (1.0f + 0.2f * levelIndex));
            }
          }
        }
      }
    }
  }
}
