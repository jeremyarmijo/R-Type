#pragma once
#include <SDL2/SDL.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class GameAction {
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  FIRE,
  SPECIAL,
  PAUSE,
  NONE
};

class KeyBindings {
 private:
  std::unordered_map<GameAction, std::vector<SDL_Scancode>> m_actionBindings;
  std::unordered_map<SDL_Scancode, GameAction> m_keyToAction;
  std::string m_configFilePath;

 public:
  KeyBindings(
      const std::string& configPath = "../Client/src/config/keybinds.cfg")
      : m_configFilePath(configPath) {
    SetDefaultBindings();
  }

  void SetDefaultBindings() {
    ClearAllBindings();

    // Movement
    BindKey(GameAction::MOVE_LEFT, SDL_SCANCODE_LEFT);
    BindKey(GameAction::MOVE_LEFT, SDL_SCANCODE_A);
    BindKey(GameAction::MOVE_RIGHT, SDL_SCANCODE_RIGHT);
    BindKey(GameAction::MOVE_RIGHT, SDL_SCANCODE_D);
    BindKey(GameAction::MOVE_UP, SDL_SCANCODE_UP);
    BindKey(GameAction::MOVE_UP, SDL_SCANCODE_W);
    BindKey(GameAction::MOVE_DOWN, SDL_SCANCODE_DOWN);
    BindKey(GameAction::MOVE_DOWN, SDL_SCANCODE_S);

    // Actions
    BindKey(GameAction::FIRE, SDL_SCANCODE_SPACE);
    // BindKey(GameAction::SPECIAL, SDL_SCANCODE_F);
    BindKey(GameAction::PAUSE, SDL_SCANCODE_ESCAPE);
    BindKey(GameAction::PAUSE, SDL_SCANCODE_P);
  }

  void BindKey(GameAction action, SDL_Scancode key) {
    UnbindKey(key);

    m_actionBindings[action].push_back(key);

    m_keyToAction[key] = action;

    std::cout << "Bound " << GetActionName(action) << " to "
              << SDL_GetScancodeName(key) << std::endl;
  }

  void UnbindKey(SDL_Scancode key) {
    auto it = m_keyToAction.find(key);
    if (it != m_keyToAction.end()) {
      GameAction action = it->second;

      auto& keys = m_actionBindings[action];
      keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());

      m_keyToAction.erase(it);
    }
  }

  void UnbindAction(GameAction action) {
    auto it = m_actionBindings.find(action);
    if (it != m_actionBindings.end()) {
      for (SDL_Scancode key : it->second) {
        m_keyToAction.erase(key);
      }
      it->second.clear();
    }
  }

  void ClearAllBindings() {
    m_actionBindings.clear();
    m_keyToAction.clear();
  }

  GameAction GetActionForKey(SDL_Scancode key) const {
    auto it = m_keyToAction.find(key);
    if (it != m_keyToAction.end()) {
      return it->second;
    }
    return GameAction::NONE;
  }

  const std::vector<SDL_Scancode>& GetKeysForAction(GameAction action) const {
    static const std::vector<SDL_Scancode> empty;
    auto it = m_actionBindings.find(action);
    if (it != m_actionBindings.end()) {
      return it->second;
    }
    return empty;
  }

  bool IsKeyBoundToAction(SDL_Scancode key, GameAction action) const {
    return GetActionForKey(key) == action;
  }

  std::string GetKeyNameForAction(GameAction action) const {
    const auto& keys = GetKeysForAction(action);
    if (keys.empty()) {
      return "Unbound";
    }

    return SDL_GetScancodeName(keys[0]);
  }

  std::string GetAllKeysForAction(GameAction action) const {
    const auto& keys = GetKeysForAction(action);
    if (keys.empty()) {
      return "Unbound";
    }

    std::string result;
    for (size_t i = 0; i < keys.size(); ++i) {
      if (i > 0) result += ", ";
      result += SDL_GetScancodeName(keys[i]);
    }
    return result;
  }

  std::string GetActionName(GameAction action) const {
    switch (action) {
      case GameAction::MOVE_LEFT:
        return "Move Left";
      case GameAction::MOVE_RIGHT:
        return "Move Right";
      case GameAction::MOVE_UP:
        return "Move Up";
      case GameAction::MOVE_DOWN:
        return "Move Down";
      case GameAction::FIRE:
        return "Fire";
      case GameAction::SPECIAL:
        return "Special";
      case GameAction::PAUSE:
        return "Pause";
      default:
        return "Unknown";
    }
  }

  bool SaveToFile() const {
    std::ofstream file(m_configFilePath);
    if (!file.is_open()) {
      std::cerr << "Failed to save keybindings to " << m_configFilePath
                << std::endl;
      return false;
    }

    for (const auto& [action, keys] : m_actionBindings) {
      for (SDL_Scancode key : keys) {
        file << static_cast<int>(action) << " " << key << "\n";
      }
    }

    file.close();
    std::cout << "Keybindings saved to " << m_configFilePath << std::endl;
    return true;
  }

  bool LoadFromFile() {
    std::ifstream file(m_configFilePath);
    if (!file.is_open()) {
      std::cout << "No keybindings file found, using defaults" << std::endl;
      return false;
    }

    ClearAllBindings();

    int actionInt;
    int keyInt;
    while (file >> actionInt >> keyInt) {
      GameAction action = static_cast<GameAction>(actionInt);
      SDL_Scancode key = static_cast<SDL_Scancode>(keyInt);
      BindKey(action, key);
    }

    file.close();
    std::cout << "Keybindings loaded from " << m_configFilePath << std::endl;
    return true;
  }
};
