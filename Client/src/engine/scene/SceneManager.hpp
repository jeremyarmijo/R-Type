#pragma once
#include <SDL2/SDL.h>

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ecs/Registry.hpp"
#include "graphics/AnimationManager.hpp"
#include "graphics/TextureManager.hpp"
#include "include/NetworkManager.hpp"
#include "inputs/InputManager.hpp"
#include "ui/UIManager.hpp"
#include "audio/AudioManager.hpp"

class GameEngine;
class SceneManager;

class SceneData {
 private:
  std::unordered_map<std::string, std::any> m_data;

 public:
  template <typename T>
  void Set(const std::string& key, const T& value) {
    m_data[key] = value;
  }

  template <typename T>
  T Get(const std::string& key, const T& defaultValue = T()) const {
    auto it = m_data.find(key);
    if (it != m_data.end()) {
      try {
        return std::any_cast<T>(it->second);
      } catch (const std::bad_any_cast&) {
        std::cerr << "SceneData: Type mismatch for key '" << key << "'"
                  << std::endl;
      }
    }
    return defaultValue;
  }

  bool Has(const std::string& key) const {
    return m_data.find(key) != m_data.end();
  }

  void Clear() { m_data.clear(); }

  void Remove(const std::string& key) { m_data.erase(key); }
};

class Scene {
 protected:
  GameEngine* m_engine;
  SceneManager* m_sceneManager;
  UIManager m_uiManager;
  std::string m_name;

 public:
  Scene(GameEngine* engine, SceneManager* sceneManager,
        const std::string& name);

  virtual ~Scene() { m_uiManager.Clear(); }
  virtual void OnEnter() = 0;
  virtual void OnExit() = 0;
  virtual void Update(float deltaTime) {}
  virtual void Render() = 0;
  virtual void HandleEvent(SDL_Event& event) {}
  const std::string& GetName() const { return m_name; }
  void QuitGame();

 protected:
  Registry& GetRegistry();
  TextureManager& GetTextures();
  AnimationManager& GetAnimations();
  InputManager& GetInput();
  NetworkManager& GetNetwork();
  AudioManager& GetAudio();
  SDL_Renderer* GetRenderer();
  Vector2 GetCameraPosition();
  UIManager& GetUI() { return m_uiManager; }
  SceneData& GetSceneData();

  void RenderSprites(int minLayer = 0, int maxLayer = 16);
  void RenderSpritesLayered();
  void ChangeScene(const std::string& sceneName);
};

class SceneManager {
 private:
  std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
  Scene* m_currentScene;
  Scene* m_nextScene;
  Registry* m_registry;
  SceneData m_sceneData;

 public:
  SceneManager()
      : m_currentScene(nullptr), m_nextScene(nullptr), m_registry(nullptr) {}

  void SetRegistry(Registry* registry) { m_registry = registry; }

  template <typename T, typename... Args>
  void RegisterScene(const std::string& name, Args&&... args) {
    if (m_scenes.find(name) != m_scenes.end()) {
      std::cerr << "Warning: Scene '" << name << "' already registered"
                << std::endl;
      return;
    }
    m_scenes[name] = std::make_unique<T>(std::forward<Args>(args)...);
    std::cout << "Registered scene: " << name << std::endl;
  }

  void ClearAllScenes() {
    if (m_currentScene) {
      m_currentScene->OnExit();
      m_currentScene = nullptr;
    }
    m_nextScene = nullptr;
    m_scenes.clear();
  }

  void ChangeScene(const std::string& name);
  void Update(float deltaTime);
  void Render();
  void HandleEvent(SDL_Event& event);

  Scene* GetCurrentScene() { return m_currentScene; }
  SceneData& GetSceneData() { return m_sceneData; }

  const std::string& GetCurrentSceneName() const {
    static const std::string none = "None";
    return m_currentScene ? m_currentScene->GetName() : none;
  }
};
