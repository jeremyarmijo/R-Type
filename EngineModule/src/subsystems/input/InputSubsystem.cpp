#include "input/InputSubsystem.hpp"

#include <iostream>
#include <string>

InputSubsystem::InputSubsystem()
    : m_keyboardState(nullptr),
      m_mouseX(0),
      m_mouseY(0),
      m_mouseState(0),
      m_gamepad(nullptr),
      m_gamepadConnected(false),
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
      m_action2(false) {}

InputSubsystem::~InputSubsystem() { Shutdown(); }

bool InputSubsystem::Initialize() {
  std::cout << "Initializing Input Subsystem..." << std::endl;

  if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
    std::cerr << "SDL gamecontroller initialization failed: " << SDL_GetError()
              << std::endl;
    return false;
  }

  m_keyboardState = SDL_GetKeyboardState(nullptr);

  if (!m_keyBindings.LoadFromFile()) {
    m_keyBindings.SetDefaultBindings();
  }

  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      ConnectGamepad();
      break;
    }
  }

  std::cout << "Input subsystem initialized" << std::endl;
  return true;
}

void InputSubsystem::Shutdown() {
  std::cout << "Shutting down Input Subsystem..." << std::endl;
  DisconnectGamepad();
}

void InputSubsystem::Update(float deltaTime) {
  m_keyPressed.clear();
  m_keyReleased.clear();
  m_mousePressed.clear();
  m_mouseReleased.clear();

  UpdateKeyboardState();
  UpdateMouseState();

  UpdateGameActions();
}

void InputSubsystem::UpdateGameActions() {
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

bool InputSubsystem::IsActionPressed(GameAction action) const {
  if (!m_keyboardState) return false;

  const auto& keys = m_keyBindings.GetKeysForAction(action);
  for (SDL_Scancode key : keys) {
    if (m_keyboardState[key]) {
      return true;
    }
  }
  return false;
}

void InputSubsystem::ProcessEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_KEYDOWN:
      if (!event.key.repeat) {
        m_keyPressed[event.key.keysym.sym] = true;
      }
      break;

    case SDL_KEYUP:
      m_keyReleased[event.key.keysym.sym] = true;
      break;

    case SDL_MOUSEBUTTONDOWN:
      m_mousePressed[event.button.button] = true;
      break;

    case SDL_MOUSEBUTTONUP:
      m_mouseReleased[event.button.button] = true;
      break;

    case SDL_CONTROLLERDEVICEADDED:
      if (!m_gamepadConnected) {
        ConnectGamepad();
      }
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
      DisconnectGamepad();
      break;
  }

  for (auto& [action, binding] : m_bindings) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == binding.key) {
      if (binding.callback) {
        binding.callback();
      }
    }
  }
}

bool InputSubsystem::IsKeyDown(SDL_Keycode key) const {
  SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
  return m_keyboardState && m_keyboardState[scancode];
}

bool InputSubsystem::IsKeyPressed(SDL_Keycode key) const {
  auto it = m_keyPressed.find(key);
  return it != m_keyPressed.end() && it->second;
}

bool InputSubsystem::IsKeyReleased(SDL_Keycode key) const {
  auto it = m_keyReleased.find(key);
  return it != m_keyReleased.end() && it->second;
}

bool InputSubsystem::IsMouseButtonDown(int button) const {
  return (m_mouseState & SDL_BUTTON(button)) != 0;
}

bool InputSubsystem::IsMouseButtonPressed(int button) const {
  auto it = m_mousePressed.find(button);
  return it != m_mousePressed.end() && it->second;
}

bool InputSubsystem::IsMouseButtonReleased(int button) const {
  auto it = m_mouseReleased.find(button);
  return it != m_mouseReleased.end() && it->second;
}

void InputSubsystem::GetMousePosition(int& x, int& y) const {
  x = m_mouseX;
  y = m_mouseY;
}

bool InputSubsystem::IsGamepadButtonDown(
    SDL_GameControllerButton button) const {
  if (!m_gamepadConnected || !m_gamepad) {
    return false;
  }
  return SDL_GameControllerGetButton(m_gamepad, button) == 1;
}

float InputSubsystem::GetGamepadAxis(SDL_GameControllerAxis axis) const {
  if (!m_gamepadConnected || !m_gamepad) {
    return 0.0f;
  }
  Sint16 value = SDL_GameControllerGetAxis(m_gamepad, axis);
  return value / 32767.0f;
}

void InputSubsystem::BindAction(const std::string& action, SDL_Keycode key,
                                std::function<void()> callback) {
  m_bindings[action] = {action, key, -1, -1, callback};
  std::cout << "Bound action '" << action << "' to key " << key << std::endl;
}

void InputSubsystem::UnbindAction(const std::string& action) {
  m_bindings.erase(action);
}

InputState InputSubsystem::GetPlayerInput() {
  InputState input;
  input.moveLeft = m_moveLeft;
  input.moveRight = m_moveRight;
  input.moveUp = m_moveUp;
  input.moveDown = m_moveDown;
  input.action1 = m_action1;
  input.action2 = m_action2;
  return input;
}

bool InputSubsystem::IsKeyScanPressed(SDL_Scancode key) const {
  return m_keyboardState && m_keyboardState[key];
}

bool InputSubsystem::WasKeyScanJustPressed(SDL_Scancode key) const {
  // Convert scancode to keycode and check
  SDL_Keycode keycode = SDL_GetKeyFromScancode(key);
  return IsKeyPressed(keycode);
}

bool InputSubsystem::WasKeyScanJustReleased(SDL_Scancode key) const {
  SDL_Keycode keycode = SDL_GetKeyFromScancode(key);
  return IsKeyReleased(keycode);
}

bool InputSubsystem::WasMoveLeftPressed() const {
  return m_moveLeft && !m_prevMoveLeft;
}

bool InputSubsystem::WasMoveLeftReleased() const {
  return !m_moveLeft && m_prevMoveLeft;
}

bool InputSubsystem::WasMoveRightPressed() const {
  return m_moveRight && !m_prevMoveRight;
}

bool InputSubsystem::WasMoveRightReleased() const {
  return !m_moveRight && m_prevMoveRight;
}

bool InputSubsystem::WasMoveUpPressed() const {
  return m_moveUp && !m_prevMoveUp;
}

bool InputSubsystem::WasMoveUpReleased() const {
  return !m_moveUp && m_prevMoveUp;
}

bool InputSubsystem::WasMoveDownPressed() const {
  return m_moveDown && !m_prevMoveDown;
}

bool InputSubsystem::WasMoveDownReleased() const {
  return !m_moveDown && m_prevMoveDown;
}

bool InputSubsystem::WasAction1Pressed() const {
  return m_action1 && !m_prevAction1;
}

bool InputSubsystem::WasAction1Released() const {
  return !m_action1 && m_prevAction1;
}

bool InputSubsystem::WasAction2Pressed() const {
  return m_action2 && !m_prevAction2;
}

bool InputSubsystem::WasAction2Released() const {
  return !m_action2 && m_prevAction2;
}

// === PRIVATE METHODS ===

void InputSubsystem::UpdateKeyboardState() {
  m_keyboardState = SDL_GetKeyboardState(nullptr);
}

void InputSubsystem::UpdateMouseState() {
  m_mouseState = SDL_GetMouseState(&m_mouseX, &m_mouseY);
}

void InputSubsystem::ConnectGamepad() {
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      m_gamepad = SDL_GameControllerOpen(i);
      if (m_gamepad) {
        m_gamepadConnected = true;
        std::cout << "Gamepad connected: " << SDL_GameControllerName(m_gamepad)
                  << std::endl;
        return;
      }
    }
  }
}

void InputSubsystem::DisconnectGamepad() {
  if (m_gamepad) {
    SDL_GameControllerClose(m_gamepad);
    m_gamepad = nullptr;
    m_gamepadConnected = false;
    std::cout << "Gamepad disconnected" << std::endl;
  }
}
#ifdef _WIN32
__declspec(dllexport) ISubsystem* CreateSubsystem() {
    return new InputSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem() { return new InputSubsystem(); }
}
#endif