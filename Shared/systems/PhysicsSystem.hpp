#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "components/Physics2D.hpp"

void physics_movement_system(Registry& registry,
                             SparseArray<Transform>& transforms,
                             SparseArray<RigidBody>& rigidbodies,
                             float deltaTime,
                             const Vector2& gravity = {0, 9.81f});
