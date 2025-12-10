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
    Entity tagger;  // celui qui touche source hit
    Entity It;
    CollisionCategory taggerType;
    CollisionCategory itType;
    //Uint32 damage;
    bool processed;
    Collision(Entity tagger, Entity It,
              CollisionCategory taggerType,
              CollisionCategory itType)
        : tagger(tagger),
          It(It),
          taggerType(taggerType),
          itType(itType),
          //damage(damage),
          processed(false) {}
};

#endif  // SHARED_COMPONENTS_COLLISION_COLLISIONCOTROLLER_HPP_
