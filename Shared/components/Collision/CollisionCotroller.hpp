// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_COLLISION_COLLISIONCOTROLLER_HPP_
    #define  SHARED_COMPONENTS_COLLISION_COLLISIONCOTROLLER_HPP_
#include "../../ecs/Entity.hpp"

enum class CollisionCategory {
    Unknown = 0,
    Player,
    Enemy,
    ProjectTile,
    Boss,
    Item,
    scene
};

struct Collision {
    Entity tagger;  // celui qui touche
    Entity It;
    CollisionCategory taggerType;
    CollisionCategory itType;
    int damage;
    bool processed;
    Collision(Entity tagger, Entity It,
              CollisionCategory taggerType,
              CollisionCategory itType,
              int damage)
        : tagger(tagger),
          It(It),
          taggerType(taggerType),
          itType(itType),
          damage(damage),
          processed(false) {}
};

enum class ItemType {
    Unknown = 0,
    HealthPotion,
    ManaPotion,
    Key,
    Coin
};

struct Items {
    ItemType type;
    int item_id;
    bool picked_up;
};

#endif  // SHARED_COMPONENTS_COLLISION_COLLISIONCOTROLLER_HPP_
