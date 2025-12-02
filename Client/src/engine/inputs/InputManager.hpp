#pragma once
#include <SDL2/SDL.h>

#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "graphics/RenderComponents.hpp"
#include "components/Physics2D.hpp"

struct PlayerControlled {
  float moveSpeed;

  explicit PlayerControlled(float speed = 200.0f) : moveSpeed(speed) {}
};

struct InputState {
  bool moveLeft;
  bool moveRight;
  bool moveUp;
  bool moveDown;
  bool action1;  // Attack, shoot, etc.
  bool action2;

  InputState()
      : moveLeft(false),
        moveRight(false),
        moveUp(false),
        moveDown(false),
        action1(false),
        action2(false) {}
};

class InputManager {
 private:
  const Uint8* m_keyboardState;
  bool m_previousJump;

 public:
  InputManager() : m_keyboardState(nullptr) {}

  void Update();

  InputState GetPlayerInput();

  bool IsKeyPressed(SDL_Scancode key) const;
};
