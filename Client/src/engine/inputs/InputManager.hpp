#pragma once
#include <SDL2/SDL.h>

#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "graphics/RenderComponents.hpp"
#include "inputs/KeyBindings.hpp"

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
  KeyBindings m_keyBindings;

  bool m_prevMoveLeft;
  bool m_prevMoveRight;
  bool m_prevMoveUp;
  bool m_prevMoveDown;
  bool m_prevAction1;
  bool m_prevAction2;

  bool m_moveLeft;
  bool m_moveRight;
  bool m_moveUp;
  bool m_moveDown;
  bool m_action1;
  bool m_action2;

 public:
  InputManager()
      : m_keyboardState(nullptr),
        m_prevMoveLeft(false),
        m_prevMoveRight(false),
        m_prevMoveUp(false),
        m_prevMoveDown(false),
        m_prevAction1(false),
        m_prevAction2(false),
        m_moveLeft(false),
        m_moveRight(false),
        m_moveUp(false),
        m_moveDown(false),
        m_action1(false),
        m_action2(false) {
    if (!m_keyBindings.LoadFromFile()) {
      m_keyBindings.SetDefaultBindings();
    }
  }

  void Update();
  InputState GetPlayerInput();
  bool IsKeyPressed(SDL_Scancode key) const;

  bool WasKeyJustPressed(SDL_Scancode key) const;
  bool WasKeyJustReleased(SDL_Scancode key) const;

  bool WasMoveLeftPressed() const;
  bool WasMoveLeftReleased() const;
  bool WasMoveRightPressed() const;
  bool WasMoveRightReleased() const;
  bool WasMoveUpPressed() const;
  bool WasMoveUpReleased() const;
  bool WasMoveDownPressed() const;
  bool WasMoveDownReleased() const;
  bool WasAction1Pressed() const;
  bool WasAction1Released() const;
  bool WasAction2Pressed() const;
  bool WasAction2Released() const;

  bool IsMoveLeftHeld() const { return m_moveLeft; }
  bool IsMoveRightHeld() const { return m_moveRight; }
  bool IsMoveUpHeld() const { return m_moveUp; }
  bool IsMoveDownHeld() const { return m_moveDown; }
  bool IsAction1Held() const { return m_action1; }
  bool IsAction2Held() const { return m_action2; }

  KeyBindings& GetKeyBindings() { return m_keyBindings; }
  const KeyBindings& GetKeyBindings() const { return m_keyBindings; }

  void SaveKeyBindings() { m_keyBindings.SaveToFile(); }

  void ResetKeyBindings() { m_keyBindings.SetDefaultBindings(); }

 private:
  bool IsActionPressed(GameAction action) const;
};
