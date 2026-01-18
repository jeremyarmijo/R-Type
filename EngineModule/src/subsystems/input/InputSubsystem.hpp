#pragma once
#include <SDL2/SDL.h>

#include <functional>
#include <string>
#include <unordered_map>

#include "engine/ISubsystem.hpp"
#include "input/KeyBindings.hpp"
#include "input/input_export.hpp"

struct InputState {
  bool moveLeft;
  bool moveRight;
  bool moveUp;
  bool moveDown;
  bool action1;
  bool action2;

  InputState()
      : moveLeft(false),
        moveRight(false),
        moveUp(false),
        moveDown(false),
        action1(false),
        action2(false) {}
};

struct InputBinding {
  std::string action;
  SDL_Keycode key;
  int mouseButton;
  int gamepadButton;
  std::function<void()> callback;
};

class INPUT_API InputSubsystem : public ISubsystem {
private:
    // Keyboard state
    const Uint8* m_keyboardState;
    std::unordered_map<SDL_Keycode, bool> m_keyPressed;
    std::unordered_map<SDL_Keycode, bool> m_keyReleased;
    
    // Mouse state
    int m_mouseX, m_mouseY;
    Uint32 m_mouseState;
    std::unordered_map<int, bool> m_mousePressed;
    std::unordered_map<int, bool> m_mouseReleased;
    
    // Gamepad support
    SDL_GameController* m_gamepad;
    bool m_gamepadConnected;
    
    // Action bindings
    std::unordered_map<std::string, InputBinding> m_bindings;
    
    // === NEW: Key Bindings System ===
    KeyBindings m_keyBindings;
    
    // Previous frame state for detecting changes
    bool m_prevMoveLeft;
    bool m_prevMoveRight;
    bool m_prevMoveUp;
    bool m_prevMoveDown;
    bool m_prevAction1;
    bool m_prevAction2;
    
    // Current frame state
    bool m_moveLeft;
    bool m_moveRight;
    bool m_moveUp;
    bool m_moveDown;
    bool m_action1;
    bool m_action2;

 public:
  InputSubsystem();
  ~InputSubsystem() override;

  bool Initialize() override;
  void Shutdown() override;
  void Update(float deltaTime) override;

  void SetRegistry(Registry* registry) override {}
  void ProcessEvent(SDL_Event event) override {}

  const char* GetName() const override { return "Input"; }
  SubsystemType GetType() const override { return SubsystemType::INPUT; }
  const char* GetVersion() const override { return "1.0.0"; }

  bool IsKeyDown(SDL_Keycode key) const;
  bool IsKeyPressed(SDL_Keycode key) const;
  bool IsKeyReleased(SDL_Keycode key) const;

  bool IsMouseButtonDown(int button) const;
  bool IsMouseButtonPressed(int button) const;
  bool IsMouseButtonReleased(int button) const;
  void GetMousePosition(int& x, int& y) const;

  bool IsGamepadButtonDown(SDL_GameControllerButton button) const;
  float GetGamepadAxis(SDL_GameControllerAxis axis) const;

  void BindAction(const std::string& action, SDL_Keycode key,
                  std::function<void()> callback);
  void UnbindAction(const std::string& action);

  void ProcessEvent(const SDL_Event& event);

  InputState GetPlayerInput();

  bool IsKeyScanPressed(SDL_Scancode key) const;
  bool WasKeyScanJustPressed(SDL_Scancode key) const;
  bool WasKeyScanJustReleased(SDL_Scancode key) const;

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
  void UpdateKeyboardState();
  void UpdateMouseState();
  void UpdateGameActions();
  void ConnectGamepad();
  void DisconnectGamepad();

  bool IsActionPressed(GameAction action) const;
};
#ifdef _WIN32
extern "C" {
__declspec(dllexport) ISubsystem* CreateSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem();
}
#endif
