#include "./Spawn.hpp"

#include "Helpers/EntityHelper.hpp"
#include "SpawnEnemy/Spawn.hpp"

void enemy_spawner_system(Registry& registry,
                          SparseArray<EnemySpawning>& spawners,
                          float deltaTime, uint8_t diff) {
  for (auto&& [idx, spawner] : IndexedZipper(spawners)) {
    spawner.timer += deltaTime;

    if (spawner.timer >= spawner.spawnInterval) {
      int currentEnemies = 0;
      auto& enemies = registry.get_components<Enemy>();
      for (auto&& [eIdx, enemy] : IndexedZipper(enemies)) {
        if (enemy.type == spawner.type) currentEnemies++;
      }

      if (currentEnemies < spawner.maxEnemiesAct) {
        Vector2 spawnPos =
            spawner.spawnPoints[rand() % spawner.spawnPoints.size()];
        createEnemy(registry, spawner.type, spawnPos, diff);
      }

      spawner.timer = 0.0f;  // reset timer
    }
  }
}
