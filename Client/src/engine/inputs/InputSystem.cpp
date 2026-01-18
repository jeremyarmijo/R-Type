#include "inputs/InputSystem.hpp"

#include <iostream>

#include "Player/PlayerEntity.hpp"
#include "include/NetworkManager.hpp"
#include "network/Action.hpp"

void player_input_system(Registry& registry,
                         SparseArray<PlayerEntity>& playerEntity,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager,
                         NetworkManager* networkManager) {
  if (!inputManager) return;

  if (networkManager) {
    PlayerInput currentInputState;
    currentInputState.left = inputManager->IsMoveLeftHeld();
    currentInputState.right = inputManager->IsMoveRightHeld();
    currentInputState.up = inputManager->IsMoveUpHeld();
    currentInputState.down = inputManager->IsMoveDownHeld();
    currentInputState.fire = inputManager->IsAction3Held();

    if (inputManager->IsAction2Held()) {
      currentInputState.fire = 2;  // Charge (touche F ou SPECIAL)
      std::cout << "[CLIENT] Sending fire=2 (FORCE TOGGLE)" << std::endl;
    } else if (inputManager->IsAction1Held()) {
      currentInputState.fire = 1;  // Tir normal (touche SPACE ou FIRE)
    } else {
      currentInputState.fire = 0;  // Rien
    }

    /*currentInputState.forceToggle = inputManager->IsForceTogglePressed();
    if (currentInputState.forceToggle) {
    }*/
    if (inputManager->m_moveLeft != inputManager->m_prevMoveLeft ||
        inputManager->m_moveRight != inputManager->m_prevMoveRight ||
        inputManager->m_moveUp != inputManager->m_prevMoveUp ||
        inputManager->m_moveDown != inputManager->m_prevMoveDown ||
        inputManager->m_action1 != inputManager->m_prevAction1 ||
        inputManager->m_action2 != inputManager->m_prevAction2 ||
        inputManager->m_action3 != inputManager->m_prevAction3) {
      Action action;
      action.type = ActionType::DOWN_PRESS;
      action.data = currentInputState;
      networkManager->SendAction(action);
    }
    /*if (currentInputState.forceToggle) {
      inputManager->ResetForceToggle();
    }*/
  }

  // client side prediction
  InputState input = inputManager->GetPlayerInput();

  for (auto&& [playerComp, transform, rigidbody] :
       Zipper(playerEntity, transforms, rigidbodies)) {
    if (rigidbody.isStatic) continue;

    // Horizontal movement
    rigidbody.velocity.x = 0;
    if (input.moveLeft) {
      rigidbody.velocity.x = -playerComp.speed;
    }
    if (input.moveRight) {
      rigidbody.velocity.x = playerComp.speed;
    }

    //  Vertical movement
    rigidbody.velocity.y = 0;
    if (input.moveUp) {
      rigidbody.velocity.y = -playerComp.speed;
    }
    if (input.moveDown) {
      rigidbody.velocity.y = playerComp.speed;
    }
  }
}
