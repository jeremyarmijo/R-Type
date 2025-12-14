#include "inputs/InputSystem.hpp"

#include <iostream>

#include "Player/PlayerEntity.hpp"
#include "include/NetworkManager.hpp"
#include "network/Action.hpp"

void player_input_system(Registry& registry,
                         SparseArray<PlayerControlled>& playerControlled,
                         SparseArray<Transform>& transforms,
                         SparseArray<RigidBody>& rigidbodies,
                         const SparseArray<BoxCollider>& colliders,
                         InputManager* inputManager,
                         NetworkManager* networkManager) {
  if (!inputManager) return;

  if (networkManager) {
    // LEFT
    if (inputManager->WasMoveLeftPressed()) {
      Action action;
      action.type = ActionType::LEFT_PRESS;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: LEFT_PRESS" << std::endl;
    }
    if (inputManager->WasMoveLeftReleased()) {
      Action action;
      action.type = ActionType::LEFT_RELEASE;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: LEFT_RELEASE" << std::endl;
    }

    // RIGHT
    if (inputManager->WasMoveRightPressed()) {
      Action action;
      action.type = ActionType::RIGHT_PRESS;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: RIGHT_PRESS" << std::endl;
    }
    if (inputManager->WasMoveRightReleased()) {
      Action action;
      action.type = ActionType::RIGHT_RELEASE;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: RIGHT_RELEASE" << std::endl;
    }

    // UP
    if (inputManager->WasMoveUpPressed()) {
      Action action;
      action.type = ActionType::UP_PRESS;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: UP_PRESS" << std::endl;
    }
    if (inputManager->WasMoveUpReleased()) {
      Action action;
      action.type = ActionType::UP_RELEASE;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: UP_RELEASE" << std::endl;
    }

    // DOWN
    if (inputManager->WasMoveDownPressed()) {
      Action action;
      action.type = ActionType::DOWN_PRESS;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: DOWN_PRESS" << std::endl;
    }
    if (inputManager->WasMoveDownReleased()) {
      Action action;
      action.type = ActionType::DOWN_RELEASE;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: DOWN_RELEASE" << std::endl;
    }

    // Action1
    if (inputManager->WasAction1Pressed()) {
      Action action;
      action.type = ActionType::FIRE_PRESS;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: FIRE_PRESS" << std::endl;
    }
    if (inputManager->WasAction1Released()) {
      Action action;
      action.type = ActionType::FIRE_RELEASE;
      action.data = std::monostate();
      networkManager->SendAction(action);
      std::cout << "Sent: FIRE_RELEASE" << std::endl;
    }
  }

  // client side prediction
  InputState input = inputManager->GetPlayerInput();

  for (auto&& [playerComp, transform, rigidbody] :
       Zipper(playerControlled, transforms, rigidbodies)) {
    if (rigidbody.isStatic) continue;

    // Horizontal movement
    rigidbody.velocity.x = 0;
    if (input.moveLeft) {
      rigidbody.velocity.x = -playerComp.speed;
    }
    if (input.moveRight) {
      rigidbody.velocity.x = playerComp.speed;
    }

    // Vertical movement
    rigidbody.velocity.y = 0;
    if (input.moveUp) {
      rigidbody.velocity.y = -playerComp.speed;
    }
    if (input.moveDown) {
      rigidbody.velocity.y = playerComp.speed;
    }
  }
}
