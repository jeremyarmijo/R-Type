#pragma once
#include "components/Physics2D.hpp"
#include "inputs/InputManager.hpp"
#include "Player/PlayerEntity.hpp"

class NetworkManager;

void player_input_system(Registry& registry,
                         SparseArray<PlayerEntity>& playerEntity,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager,
                         NetworkManager* networkManager = nullptr);
