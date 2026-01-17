#pragma once
#include "ecs/Registry.hpp"
#include "input/InputSubsystem.hpp"
#include "network/NetworkSubsystem.hpp"
#include "Player/PlayerEntity.hpp"
#include "systems/InputSystem.hpp"

// Functional player input system
void player_input_system(Registry& registry,
                                InputSubsystem* input,
                                NetworkSubsystem* network) {
    if (!input) return;
    
    auto& playerEntities = registry.get_components<PlayerEntity>();
    auto& transforms = registry.get_components<Transform>();
    auto& rigidbodies = registry.get_components<RigidBody>();
    
    // Send network input if connected
    if (network) {
        // Check if input state changed
        if (input->WasMoveLeftPressed() || input->WasMoveLeftReleased() ||
            input->WasMoveRightPressed() || input->WasMoveRightReleased() ||
            input->WasMoveUpPressed() || input->WasMoveUpReleased() ||
            input->WasMoveDownPressed() || input->WasMoveDownReleased() ||
            input->WasAction1Pressed() || input->WasAction1Released()) {
            
            PlayerInput currentInputState;
            currentInputState.left = input->IsMoveLeftHeld();
            currentInputState.right = input->IsMoveRightHeld();
            currentInputState.up = input->IsMoveUpHeld();
            currentInputState.down = input->IsMoveDownHeld();
            currentInputState.fire = input->IsAction1Held();
            
            Action action;
            action.type = ActionType::DOWN_PRESS;
            action.data = currentInputState;
            network->SendAction(action);
        }
    }
    
    // Client-side prediction
    InputState inputState = input->GetPlayerInput();
    
    for (size_t i = 0; i < std::min({playerEntities.size(), transforms.size(), rigidbodies.size()}); ++i) {
        if (!playerEntities[i].has_value() || !transforms[i].has_value() || 
            !rigidbodies[i].has_value()) {
            continue;
        }
        
        auto& player = playerEntities[i].value();
        auto& rb = rigidbodies[i].value();
        
        if (rb.isStatic) continue;
        
        // Horizontal movement
        rb.velocity.x = 0;
        if (inputState.moveLeft) {
            rb.velocity.x = -player.speed;
        }
        if (inputState.moveRight) {
            rb.velocity.x = player.speed;
        }
        
        // Vertical movement
        rb.velocity.y = 0;
        if (inputState.moveUp) {
            rb.velocity.y = -player.speed;
        }
        if (inputState.moveDown) {
            rb.velocity.y = player.speed;
        }
    }
}
