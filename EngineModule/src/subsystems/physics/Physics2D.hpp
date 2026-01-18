#pragma once
#include <SDL2/SDL.h>

#include <cmath>
#include <algorithm>

#include "ecs/Entity.hpp"

struct Vector2 {
  float x, y;

  Vector2(float x = 0, float y = 0) : x(x), y(y) {}  // NOLINT(runtime/explicit)

  Vector2 operator+(const Vector2& other) const {
    return {x + other.x, y + other.y};
  }

  Vector2 operator-(const Vector2& other) const {
    return {x - other.x, y - other.y};
  }

  Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

  Vector2& operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  float Length() const { return std::sqrt(x * x + y * y); }

  Vector2 Normalized() const {
    float len = Length();
    return len > 0 ? Vector2(x / len, y / len) : Vector2();
  }
};

struct Transform {
  Vector2 position;
  Vector2 scale;
  float rotation;

  Transform(Vector2 pos = {0, 0}, Vector2 scl = {1, 1}, float rot = 0)
      : position(pos), scale(scl), rotation(rot) {}
};

struct RigidBody {
  Vector2 velocity;
  Vector2 acceleration;
  float mass;
  float restitution;  // Bounciness (0 = no bounce, 1 = perfect bounce)
  bool isStatic;      // Static objects don't move

  explicit RigidBody(float m = 1.0f, float rest = 0.5f, bool stat = false)
      : velocity{0, 0},
        acceleration{0, 0},
        mass(m),
        restitution(rest),
        isStatic(stat) {}
};

struct CollisionEvent {
  Entity entityA;
  Entity entityB;
  Vector2 point;
  Vector2 normal;
};

enum CollisionLayer {
  LAYER_PLAYER = 1 << 0,
  LAYER_ENEMY = 1 << 1,
  LAYER_PROJECTILE_PLAYER = 1 << 2,
  LAYER_PROJECTILE_ENEMY = 1 << 3,
  LAYER_WORLD = 1 << 4,
  LAYER_PICKUP = 1 << 5
};

struct BoxCollider {
  float width, height;
  Vector2 offset;
  uint32_t layer;
  uint32_t mask;   // What layers this collider interacts with
  bool isTrigger;  // Trigger = no physical response, just events

  BoxCollider(float w, float h, Vector2 off = {0, 0},
              uint32_t lay = LAYER_WORLD, uint32_t msk = 0xFFFFFFFF,
              bool trigger = false)
      : width(w),
        height(h),
        offset(off),
        layer(lay),
        mask(msk),
        isTrigger(trigger) {}

  SDL_Rect GetRect(const Vector2& position) const {
    return {static_cast<int>(position.x + offset.x - width / 2),
            static_cast<int>(position.y + offset.y - height / 2),
            static_cast<int>(width), static_cast<int>(height)};
  }

  struct Bounds {
    float left, right, top, bottom;
  };

  Bounds GetBounds(const Vector2& position) const {
    return {
        position.x + offset.x - width / 2,   // left
        position.x + offset.x + width / 2,   // right
        position.y + offset.y - height / 2,  // top
        position.y + offset.y + height / 2   // bottom
    };
  }
};
