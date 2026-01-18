// Copyright 2025 Dalia Guiz
#ifndef SHARED_COMPONENTS_FORCE_HPP_
#define SHARED_COMPONENTS_FORCE_HPP_

#include "physics/Physics2D.hpp"
#include "ecs/Entity.hpp"

enum class EForceState { AttachedFront, AttachedBack, Detached };

struct Force {
  Entity ownerPlayer;
  EForceState state;
  Vector2 direction;
  Vector2 offsetFront;
  Vector2 offsetBack;
  Vector2 detachPosition;
  float speed;
  float maxDistance;
  float currentDistance;
  int contactDamage;
  bool blocksProjectiles;
  bool isActive;

  Force(Entity owner = Entity(0), EForceState st = EForceState::AttachedFront,
        float spd = 200.f, Vector2 dir = {1.f, 0.f},
        Vector2 offFront = {20.f, -30.f}, Vector2 offBack = {-20.f, -30.f},
        float maxDist = 300.f, int dmg = 20, bool blocks = true)
      : ownerPlayer(owner),
        state(st),
        direction(dir),
        offsetFront(offFront),
        offsetBack(offBack),
        detachPosition({0.f, 0.f}),
        speed(spd),
        maxDistance(maxDist),
        currentDistance(0.f),
        contactDamage(dmg),
        blocksProjectiles(blocks),
        isActive(true) {}
};

#endif  // SHARED_COMPONENTS_FORCE_HPP_
