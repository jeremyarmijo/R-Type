#pragma once
#include "inputs/InputManager.hpp"
#include "physics/Physics2D.hpp"

void player_input_system(Registry& registry,
                         SparseArray<PlayerControlled>& playerControlled,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager);
