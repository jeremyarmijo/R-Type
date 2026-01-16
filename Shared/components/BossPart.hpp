#ifndef SHARED_COMPONENTS_BOSSPART_HPP_
#define SHARED_COMPONENTS_BOSSPART_HPP_

#include "components/Physics2D.hpp"
#include "ecs/Entity.hpp"

struct BossPart {
  Entity bossEntity;  // Le boss parent
  Vector2 offset;     // Position relative
  int segmentIndex;   // Pour snake : 0,1,2... (-1 si tourelle fixe)
  float timeOffset;   // Pour snake : décalage temporel (0 si tourelle)
  int hp;
  bool alive;
  uint8_t partType;  // ← AJOUTE ÇA : 0=serpent_body, 1=turret, etc.
  float timer = 0.f;

  BossPart(Entity boss = Entity(0), Vector2 off = {0.f, 0.f}, int index = -1,
           float tOffset = 0.f, int health = 50, uint8_t pType = 0)
      : bossEntity(boss),
        offset(off),
        segmentIndex(index),
        timeOffset(tOffset),
        hp(health),
        alive(true),
        partType(pType) {}
};

#endif  // SHARED_COMPONENTS_BOSSPART_HPP_
