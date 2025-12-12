#include "inputs/InputManager.hpp"

#include <iostream>

bool InputManager::IsActionPressed(GameAction action) const {
  if (!m_keyboardState) return false;

  const auto& keys = m_keyBindings.GetKeysForAction(action);
  for (SDL_Scancode key : keys) {
    if (m_keyboardState[key]) {
      return true;
    }
  }
  return false;
}

void InputManager::Update() {
  m_keyboardState = SDL_GetKeyboardState(nullptr);
  
  if (!m_keyboardState) return;

  m_prevMoveLeft = m_moveLeft;
  m_prevMoveRight = m_moveRight;
  m_prevMoveUp = m_moveUp;
  m_prevMoveDown = m_moveDown;
  m_prevAction1 = m_action1;
  m_prevAction2 = m_action2;

  m_moveLeft = IsActionPressed(GameAction::MOVE_LEFT);
  m_moveRight = IsActionPressed(GameAction::MOVE_RIGHT);
  m_moveUp = IsActionPressed(GameAction::MOVE_UP);
  m_moveDown = IsActionPressed(GameAction::MOVE_DOWN);
  m_action1 = IsActionPressed(GameAction::FIRE);
  m_action2 = IsActionPressed(GameAction::SPECIAL);
}

InputState InputManager::GetPlayerInput() {
  InputState input;
  
  input.moveLeft = m_moveLeft;
  input.moveRight = m_moveRight;
  input.moveUp = m_moveUp;
  input.moveDown = m_moveDown;
  input.action1 = m_action1;
  input.action2 = m_action2;

  return input;
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const {
  return m_keyboardState && m_keyboardState[key];
}

bool InputManager::WasMoveLeftPressed() const {
  return m_moveLeft && !m_prevMoveLeft;
}

bool InputManager::WasMoveLeftReleased() const {
  return !m_moveLeft && m_prevMoveLeft;
}

bool InputManager::WasMoveRightPressed() const {
  return m_moveRight && !m_prevMoveRight;
}

bool InputManager::WasMoveRightReleased() const {
  return !m_moveRight && m_prevMoveRight;
}

bool InputManager::WasMoveUpPressed() const {
  return m_moveUp && !m_prevMoveUp;
}

bool InputManager::WasMoveUpReleased() const {
  return !m_moveUp && m_prevMoveUp;
}

bool InputManager::WasMoveDownPressed() const {
  return m_moveDown && !m_prevMoveDown;
}

bool InputManager::WasMoveDownReleased() const {
  return !m_moveDown && m_prevMoveDown;
}

bool InputManager::WasAction1Pressed() const {
  return m_action1 && !m_prevAction1;
}

bool InputManager::WasAction1Released() const {
  return !m_action1 && m_prevAction1;
}

bool InputManager::WasAction2Pressed() const {
  return m_action2 && !m_prevAction2;
}

bool InputManager::WasAction2Released() const {
  return !m_action2 && m_prevAction2;
}
