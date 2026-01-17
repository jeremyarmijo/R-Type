#pragma once
#include <cstdint>

enum DataMask : uint16_t {
  M_NONE = 0,
  // Commun
  M_POS_X = 1 << 0,
  M_POS_Y = 1 << 1,
  M_HP = 1 << 2,
  M_STATE = 1 << 3,

  // Player
  M_SHIELD = 1 << 4,
  M_WEAPON = 1 << 5,
  M_SPRITE = 1 << 6,
  M_SCORE = 1 << 12,

  //  Enemy / Projectile
  M_TYPE = 1 << 7,
  M_DIR = 1 << 8,
  M_VELOCITY = 1 << 9,
  M_OWNER = 1 << 10,
  M_DAMAGE = 1 << 11,

  // destruction
  M_DELETE = 1 << 15
};
