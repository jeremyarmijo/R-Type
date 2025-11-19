#include "InputSystem.hpp"

#include <iostream>

void player_input_system(Registry& registry,
                         SparseArray<PlayerControlled>& playerControlled,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager) {
  if (!inputManager) return;

  InputState input = inputManager->GetPlayerInput();

  for (auto&& [playerComp, transform, rigidbody] :
       Zipper(playerControlled, transforms, rigidbodies)) {
    if (rigidbody.isStatic) continue;

    // Horizontal movement
    rigidbody.velocity.x = 0;
    if (input.moveLeft) {
      rigidbody.velocity.x = -playerComp.moveSpeed;
    }
    if (input.moveRight) {
      rigidbody.velocity.x = playerComp.moveSpeed;
    }

    // Vertical movement
    rigidbody.velocity.y = 0;
    if (input.moveUp) {
      rigidbody.velocity.y = -playerComp.moveSpeed;
    }
    if (input.moveDown) {
      rigidbody.velocity.y = playerComp.moveSpeed;
    }
  }
}
