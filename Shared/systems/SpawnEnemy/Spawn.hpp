#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "Player/EnemySpawn.hpp"

void enemy_spawner_system(Registry& registry,
                          SparseArray<EnemySpawning>& spawners,
                          float deltaTime);