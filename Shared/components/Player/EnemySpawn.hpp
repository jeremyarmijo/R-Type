#pragma once
#include <vector>
#include "./Enemy.hpp"

struct EnemySpawning {
    std::vector<Vector2> spawnPoints;
    float spawnInterval;              
    float timer = 0.0f;  
    int maxEnemiesAct;                   
    EnemyType type;
    int spawnAmount;            
    bool waveActive = false;       
};