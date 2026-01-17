#pragma once

#include "ecs/Registry.hpp"
#include "components/TileMap.hpp"
#include "components/Physics2D.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Enemy.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/Zipper.hpp"

// Système de collision tilemap pour les joueurs
void tilemap_collision_system(Registry& registry) {
    auto& tilemaps = registry.get_components<TileMap>();
    auto& transforms = registry.get_components<Transform>();
    auto& players = registry.get_components<PlayerEntity>();
    auto& colliders = registry.get_components<BoxCollider>();
    
    // Trouver la tilemap active
    TileMap* activeTilemap = nullptr;
    for (auto& tm : tilemaps) {
        if (tm.has_value() && tm->tiles.size() > 0) {
            activeTilemap = &tm.value();
            break;
        }
    }
    
    if (!activeTilemap) return;
    
    // Vérifier collisions pour chaque joueur
    for (auto&& [idx, player, transform, collider] : 
         IndexedZipper(players, transforms, colliders)) {
        
        if (!player.isAlive) continue;
        
        // Utiliser width/height du BoxCollider
        float halfW = collider.width / 2.0f;
        float halfH = collider.height / 2.0f;
        
        // Points de collision (coins de la hitbox)
        float left = transform.position.x - halfW;
        float right = transform.position.x + halfW;
        float top = transform.position.y - halfH;
        float bottom = transform.position.y + halfH;
        
        // Collision avec le SOL (bas)
        if (activeTilemap->isSolidAtPixel(transform.position.x, bottom)) {
            // Repousser vers le haut
            int tileY = static_cast<int>(bottom / activeTilemap->tileSize);
            float tileTop = static_cast<float>(tileY * activeTilemap->tileSize);
            transform.position.y = tileTop - halfH;
        }
        
        // Collision avec le PLAFOND (haut)
        if (activeTilemap->isSolidAtPixel(transform.position.x, top)) {
            // Repousser vers le bas
            int tileY = static_cast<int>(top / activeTilemap->tileSize);
            float tileBottom = static_cast<float>((tileY + 1) * activeTilemap->tileSize);
            transform.position.y = tileBottom + halfH;
        }
        
        // Collision avec MUR GAUCHE
        if (activeTilemap->isSolidAtPixel(left, transform.position.y)) {
            int tileX = static_cast<int>(left / activeTilemap->tileSize);
            float tileRight = static_cast<float>((tileX + 1) * activeTilemap->tileSize);
            transform.position.x = tileRight + halfW;
        }
        
        // Collision avec MUR DROIT
        if (activeTilemap->isSolidAtPixel(right, transform.position.y)) {
            int tileX = static_cast<int>(right / activeTilemap->tileSize);
            float tileLeft = static_cast<float>(tileX * activeTilemap->tileSize);
            transform.position.x = tileLeft - halfW;
        }
    }
}
void tilemap_enemy_collision_system(Registry& registry) {
    auto& tilemaps = registry.get_components<TileMap>();
    auto& transforms = registry.get_components<Transform>();
    auto& enemies = registry.get_components<Enemy>();
    auto& colliders = registry.get_components<BoxCollider>();

    // Trouver la tilemap active
    TileMap* activeTilemap = nullptr;
    for (auto& tm : tilemaps) {
        if (tm.has_value() && tm->tiles.size() > 0) {
            activeTilemap = &tm.value();
            break;
        }
    }
    if (!activeTilemap) return;

    // Parcourir tous les ennemis
    for (auto&& [idx, enemy, transform, collider] :
         IndexedZipper(enemies, transforms, colliders)) {

        float halfH = collider.height / 2.0f;
        float halfW = collider.width / 2.0f;
        float bottom = transform.position.y + halfH;
        float top    = transform.position.y - halfH;
        float left   = transform.position.x - halfW;
        float right  = transform.position.x + halfW;

        // --- Collision SOL ---
        if (activeTilemap->isSolidAtPixel(left, bottom) || activeTilemap->isSolidAtPixel(right, bottom)) {
            int tileY = static_cast<int>(bottom / activeTilemap->tileSize);
            float tileTop = static_cast<float>(tileY * activeTilemap->tileSize);
            transform.position.y = tileTop - halfH;

            // Pour mini_Green : rebond ou saut
            if (enemy.type == EnemyType::mini_Green) {
                enemy.direction.y = -1.f; // saute vers le haut
            } else {
                enemy.direction.y = 0.f;  // stop vertical movement
            }
        }

        // --- Collision PLAFOND ---
        if (activeTilemap->isSolidAtPixel(left, top) || activeTilemap->isSolidAtPixel(right, top)) {
            int tileY = static_cast<int>(top / activeTilemap->tileSize);
            float tileBottom = static_cast<float>((tileY + 1) * activeTilemap->tileSize);
            transform.position.y = tileBottom + halfH;
            enemy.direction.y = 0.f;
        }

        // --- Collision MUR GAUCHE ---
        if (activeTilemap->isSolidAtPixel(left, top) || activeTilemap->isSolidAtPixel(left, bottom)) {
            int tileX = static_cast<int>(left / activeTilemap->tileSize);
            float tileRight = static_cast<float>((tileX + 1) * activeTilemap->tileSize);
            transform.position.x = tileRight + halfW;
            enemy.direction.x = 0.f;
        }

        // --- Collision MUR DROIT ---
        if (activeTilemap->isSolidAtPixel(right, top) || activeTilemap->isSolidAtPixel(right, bottom)) {
            int tileX = static_cast<int>(right / activeTilemap->tileSize);
            float tileLeft = static_cast<float>(tileX * activeTilemap->tileSize);
            transform.position.x = tileLeft - halfW;
            enemy.direction.x = 0.f;
        }
    }
}
