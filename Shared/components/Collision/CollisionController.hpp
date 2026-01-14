// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_COLLISION_COLLISIONCONTROLLER_HPP_
#define SHARED_COMPONENTS_COLLISION_COLLISIONCONTROLLER_HPP_

#include "../../ecs/Entity.hpp"

enum class CollisionCategory {
  Unknown = 0,
  Player,
  Enemy,
  Projectile,
  Boss,
  BossPart,
  Force,
  Item,
  scene
};

struct Collision {
  Entity tagger;  // celui qui touche source hit
  Entity It;
  CollisionCategory taggerType;
  CollisionCategory itType;
  bool processed;
  Collision(Entity tagger, Entity It, CollisionCategory taggerType,
            CollisionCategory itType)
      : tagger(tagger),
        It(It),
        taggerType(taggerType),
        itType(itType),
        processed(false) {}
};

#endif  // SHARED_COMPONENTS_COLLISION_COLLISIONCONTROLLER_HPP_
