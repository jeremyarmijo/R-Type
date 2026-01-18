// Chemin: Server/src/systems/MapGenerator.hpp
#pragma once
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>

#include "components/TileMap.hpp"

class MapGenerator {
 public:
  // Génère une map simple avec sol en bas
  static TileMap generateLevel(int levelIndex, uint16_t screenWidth = 800,
                               uint16_t screenHeight = 600) {
    TileMap map;

    const uint16_t tileSize = 32;
    const uint16_t tilesPerScreen = screenWidth / tileSize;  // 25 tiles
    const uint16_t mapWidthInScreens = 8;  // 8 écrans de large

    uint16_t mapWidth = tilesPerScreen * mapWidthInScreens;  // 200 tiles
    uint16_t mapHeight = screenHeight / tileSize;            // 18-19 tiles

    map.init(mapWidth, mapHeight, tileSize);
    map.scrollSpeed =
        50.0f + (levelIndex * 10.0f);  // Plus rapide à chaque niveau

    // Générer le sol (2 dernières lignes)
    generateGround(map, levelIndex);

    // Générer le plafond (optionnel, 1 ligne en haut)
    generateCeiling(map, levelIndex);

    // Ajouter des obstacles selon le niveau
    generateObstacles(map, levelIndex);

    return map;
  }

 private:
  static void generateGround(TileMap& map, int levelIndex) {
    // Sol = les 2 dernières lignes
    int groundStartY = map.height - 2;

    for (int x = 0; x < map.width; ++x) {
      // Sol de base
      map.setTile(x, groundStartY, TileType::GROUND);
      map.setTile(x, groundStartY + 1, TileType::GROUND);
    }

    // Ajouter des trous dans le sol (niveau 2+)
    if (levelIndex >= 1) {
      int numHoles = 3 + levelIndex * 2;

      for (int i = 0; i < numHoles; ++i) {
        int holeX = 30 + (rand() % (map.width - 60));
        int holeWidth = 2 + (rand() % 3);
        for (int dx = 0; dx < holeWidth; ++dx) {
          map.setTile(holeX + dx, groundStartY, TileType::EMPTY);
          map.setTile(holeX + dx, groundStartY + 1, TileType::EMPTY);
        }
      }
    }
  }

  static void generateCeiling(TileMap& map, int levelIndex) {
    // Plafond = première ligne (optionnel selon niveau)
    if (levelIndex >= 1) {
      for (int x = 0; x < map.width; ++x) {
        map.setTile(x, 0, TileType::CEILING);
      }
    }
  }

  static void generateObstacles(TileMap& map, int levelIndex) {
    if (levelIndex < 1) return;  // Pas d'obstacles au niveau 1

    int numObstacles = 2 + levelIndex * 3;

    for (int i = 0; i < numObstacles; ++i) {
      int obsX = 40 + (rand() % (map.width - 80));
      int obsY =
          4 + (rand() % (map.height - 8));  // Pas trop haut ni trop bas
      int obsWidth = 1 + (rand() % 2);
      int obsHeight = 2 + (rand() % 3);

      for (int dx = 0; dx < obsWidth; ++dx) {
        for (int dy = 0; dy < obsHeight; ++dy) {
          map.setTile(obsX + dx, obsY + dy, TileType::WALL);
        }
      }
    }
  }
};
