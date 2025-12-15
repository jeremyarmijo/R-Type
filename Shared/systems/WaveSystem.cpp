// #pragma once

#include "ecs/Zipper.hpp"
#include "SpawnEnemy/Spawn.hpp"
#include "Helpers/EntityHelper.hpp"


Vector2 get_random_pos() {
  Vector2 pos;

  pos.x = rand() % (750-500 + 1) + 500;
  pos.y = rand() % (550-30 + 1) + 30;
  return pos;
}

bool checkWaveEnd(Registry& registry, SparseArray<Enemy>& enemies) {
    if (enemies.size() == 0)
      return true;
    return false;
}

void create_multiples_enemies(Registry& registry, EnemyType type, int nbEnemies) {

    for (int i = 0; i <= nbEnemies; i++) {
      Vector2 pos = get_random_pos();
      createEnemy(registry, type, pos);
    }
}

void enemy_wave_system(Registry& registry, SparseArray<Enemy>& enemies,
                            float deltaTime, int nbWave, int difficulty) {
    int i = 0;
    bool waveEnded = false;


    while (i <= nbWave) {
      int nbBasic = ((3 + i) / 2) * difficulty;
      int nbZigzag = ((2 + i) / 2) * difficulty;
      int nbWave = ((1 + i) / 2) * difficulty;

      std::vector<Vector2> pos;
      
      create_multiples_enemies(registry, EnemyType::Basic, nbBasic);
      create_multiples_enemies(registry, EnemyType::Zigzag, nbZigzag);
      create_multiples_enemies(registry, EnemyType::Wave, nbWave);

      waveEnded = checkWaveEnd(registry, enemies);

      if (waveEnded)
        i++;
    }
}
