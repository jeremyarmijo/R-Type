#include "inputs/InputManager.hpp"

#include <iostream>

void InputManager::Update() { m_keyboardState = SDL_GetKeyboardState(nullptr); }

InputState InputManager::GetPlayerInput() {
  InputState input;

  if (!m_keyboardState) return input;

  // Movement
  input.moveLeft =
      m_keyboardState[SDL_SCANCODE_LEFT] || m_keyboardState[SDL_SCANCODE_A];
  input.moveRight =
      m_keyboardState[SDL_SCANCODE_RIGHT] || m_keyboardState[SDL_SCANCODE_D];
  input.moveUp =
      m_keyboardState[SDL_SCANCODE_UP] || m_keyboardState[SDL_SCANCODE_W];
  input.moveDown =
      m_keyboardState[SDL_SCANCODE_DOWN] || m_keyboardState[SDL_SCANCODE_S];

  // Actions
  input.action1 =
      m_keyboardState[SDL_SCANCODE_D] || m_keyboardState[SDL_SCANCODE_J];
  input.action2 =
      m_keyboardState[SDL_SCANCODE_F] || m_keyboardState[SDL_SCANCODE_K];

  return input;
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const {
  return m_keyboardState && m_keyboardState[key];
}
