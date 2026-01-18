// Chemin: shared/components/TileMap.hpp
#pragma once
#include <cstdint>
#include <vector>
#include "physics/Physics2D.hpp"  // Pour Vector2

enum class TileType : uint8_t {
    EMPTY = 0,
    GROUND = 1,
    WALL = 2,
    CEILING = 3,
    PLATFORM = 4,
};

struct TileMap {
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t tileSize = 32;
    float scrollSpeed = 50.0f;
    float scrollOffset = 0.0f;
    std::vector<uint8_t> tiles;
    bool isLoaded = false;
    
    TileType getTile(int x, int y) const {
        if (x < 0 || x >= static_cast<int>(width) || 
            y < 0 || y >= static_cast<int>(height)) {
            return TileType::EMPTY;
        }
        return static_cast<TileType>(tiles[y * width + x]);
    }
    
    void setTile(int x, int y, TileType type) {
        if (x >= 0 && x < static_cast<int>(width) && 
            y >= 0 && y < static_cast<int>(height)) {
            tiles[y * width + x] = static_cast<uint8_t>(type);
        }
    }
    
    TileType getTileAtPixel(float pixelX, float pixelY) const {
        int tileX = static_cast<int>((pixelX + scrollOffset) / tileSize);
        int tileY = static_cast<int>(pixelY / tileSize);
        return getTile(tileX, tileY);
    }
    
    bool isSolid(int tileX, int tileY) const {
        TileType type = getTile(tileX, tileY);
        return type == TileType::GROUND || 
               type == TileType::WALL || 
               type == TileType::CEILING ||
               type == TileType::PLATFORM;
    }
    
    bool isSolidAtPixel(float pixelX, float pixelY) const {
        int tileX = static_cast<int>((pixelX + scrollOffset) / tileSize);
        int tileY = static_cast<int>(pixelY / tileSize);
        return isSolid(tileX, tileY);
    }
    
    // Collision avec une BoxCollider
    bool checkCollision(const Vector2& position, const BoxCollider& collider) const {
        auto bounds = collider.GetBounds(position);
        
        return isSolidAtPixel(bounds.left, bounds.top) ||
               isSolidAtPixel(bounds.right, bounds.top) ||
               isSolidAtPixel(bounds.left, bounds.bottom) ||
               isSolidAtPixel(bounds.right, bounds.bottom);
    }
    
    void init(uint16_t w, uint16_t h, uint16_t tSize = 32) {
        width = w;
        height = h;
        tileSize = tSize;
        scrollOffset = 0.0f;
        tiles.resize(w * h, static_cast<uint8_t>(TileType::EMPTY));
    }
    
    void update(float deltaTime) {
        scrollOffset += scrollSpeed * deltaTime;
    }
};