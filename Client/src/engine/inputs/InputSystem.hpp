#pragma once
#include "Player/PlayerEntity.hpp"
#include "components/Physics2D.hpp"
#include "inputs/InputManager.hpp"

class NetworkManager;

void player_input_system(Registry& registry,
                         SparseArray<PlayerControlled>& playerControlled,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager,
                         NetworkManager* networkManager = nullptr);
