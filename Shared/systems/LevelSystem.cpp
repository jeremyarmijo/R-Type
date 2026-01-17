#include "systems/LevelSystem.hpp"

#include <iostream>
#include <vector>

#include "Helpers/EntityHelper.hpp"
#include "Player/Boss.hpp"
#include "components/Levels.hpp"
#include "components/Player/Enemy.hpp"
#include "ecs/Zipper.hpp"

const float TIME_BETWEEN_WAVES = 3.0f;

inline std::vector<LevelComponent> createLevels() {
  std::vector<LevelComponent> levels;

  // Level 1
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
      Wave{{}, {0}, {{700, 250}}, true, BossType::FinalBoss, 300}});

  // Level 2
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
      Wave{{}, {0}, {{700, 250}}, true, BossType::Gomander_snake, 300}});

  // Level 3
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
      Wave{{}, {0}, {{700, 300}}, true, BossType::Bydo_Battleship, 1200}});

  return levels;
}

bool update_level_system(Registry& registry, float deltaTime, int levelIndex) {
  auto& levels = registry.get_components<LevelComponent>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();
  LevelComponent* currentLevel = nullptr;

  for (auto& lvl : levels) {
    if (lvl.has_value()) {
      currentLevel = &lvl.value();
      break;
    }
  }

  if (!currentLevel) {
    return false;
  }

  LevelComponent& level = *currentLevel;

  if (level.finishedLevel) return true;

  if (!level.initialized) {
    std::cout << "[Level " << (levelIndex + 1) << "] Started!" << std::endl;
    level.initialized = true;
  }

  bool anyEnemyAlive = false;
  for (auto& en : enemies) {
    if (en.has_value()) {
      anyEnemyAlive = true;
      break;
    }
  }

  if (!anyEnemyAlive) {
    for (auto& bo : bosses) {
      if (bo.has_value()) {
        anyEnemyAlive = true;
        break;
      }
    }
  }

  if (anyEnemyAlive) {
    return false;
  }

  level.waveDelayTimer += deltaTime;

  if (level.waveDelayTimer < TIME_BETWEEN_WAVES) {
    return false;
  }

  level.waveDelayTimer = 0.0f;
  level.currentWave++;

  if (level.currentWave >= static_cast<int>(level.waves.size())) {
    level.finishedLevel = true;
    std::cout << "[Level " << (levelIndex + 1) << "] Completed!" << std::endl;
    return true;
  }

  Wave& wave = level.waves[level.currentWave];
  std::cout << "[Level " << (levelIndex + 1) << "] Wave "
            << (level.currentWave + 1) << "/" << level.waves.size()
            << " starting!" << std::endl;

  float speedMultiplier = 1.0f + 0.15f * static_cast<float>(levelIndex);

  if (wave.isBossWave) {
    BossType btype = wave.bossType.value_or(BossType::BigShip);
    Vector2 bossPos = wave.spawnPositions.empty() ? Vector2{700.0f, 300.0f}
                                                  : wave.spawnPositions[0];
    int bossHP = (wave.bossHP > 0) ? wave.bossHP : 500 + levelIndex * 200;

    Entity bossEntity =
        createBoss(registry, btype, bossPos, BossPhase::Phase1, bossHP);

    if (btype == BossType::Gomander_snake) {
      for (int i = 0; i < 8; ++i) {
        Vector2 segmentPos = {bossPos.x + (i + 1) * 40.f, bossPos.y};
        createBossPart(registry, bossEntity, segmentPos, {0.f, 0.f}, i,
                       (i + 1) * 0.15f, 50);
      }
      std::cout << "[Level " << (levelIndex + 1)
                << "] Snake Boss spawned with 8 segments!" << std::endl;
    } else if (btype == BossType::Bydo_Battleship) {
      std::vector<Vector2> offsets = {
          {-20.f, -60.f}, {-20.f, 60.f}, {-50.f, -30.f}, {-50.f, 30.f}};
      for (const auto& offset : offsets) {
        Vector2 turretPos = {bossPos.x + offset.x, bossPos.y + offset.y};
        createBossPart(registry, bossEntity, turretPos, offset, -1, 0.f, 100,
                       {20.f, 20.f});
      }
      std::cout << "[Level " << (levelIndex + 1)
                << "] Final Boss spawned with 4 turrets!" << std::endl;
    } else {
      std::cout << "[Level " << (levelIndex + 1) << "] Boss spawned!"
                << std::endl;
    }
    auto& bossArr = registry.get_components<Boss>();
    size_t id = static_cast<size_t>(bossEntity);
    if (id < bossArr.size() && bossArr[id].has_value()) {
      bossArr[id]->speed *= speedMultiplier;
    }

  } else {
    int totalSpawned = 0;
    for (size_t t = 0; t < wave.enemyTypes.size(); ++t) {
      EnemyType type = wave.enemyTypes[t];
      int count = (t < wave.enemiesPerType.size()) ? wave.enemiesPerType[t] : 1;

      for (int k = 0; k < count; ++k) {
        Vector2 spawnPos =
            wave.spawnPositions.empty()
                ? Vector2{750.0f,
                          static_cast<float>(50 + (totalSpawned * 50) % 500)}
                : wave.spawnPositions[totalSpawned %
                                      wave.spawnPositions.size()];

        Entity e = createEnemy(registry, type, spawnPos);

        auto& enemiesArr = registry.get_components<Enemy>();
        size_t eid = static_cast<size_t>(e);
        if (eid < enemiesArr.size() && enemiesArr[eid].has_value()) {
          enemiesArr[eid]->speed *= speedMultiplier;
          enemiesArr[eid]->current =
              static_cast<int>(enemiesArr[eid]->current *
                               (1.0f + 0.2f * static_cast<float>(levelIndex)));
        }
        totalSpawned++;
      }
    }
    std::cout << "[Level " << (levelIndex + 1) << "] Spawned " << totalSpawned
              << " enemies" << std::endl;
  }

  return false;
}
