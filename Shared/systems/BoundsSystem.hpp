#pragma once
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"

void bounds_check_system(Registry& registry, SparseArray<Transform>& transforms,
                         SparseArray<BoxCollider>& colliders,
                         SparseArray<RigidBody>& rigidbodies);
