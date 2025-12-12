#pragma once
#include <SDL2/SDL.h>

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

class GameEngine;
class SceneManager;

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
  SDL_Renderer* GetRenderer();
  Vector2 GetCameraPosition();
  UIManager& GetUI() { return m_uiManager; }

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
  const std::string& GetCurrentSceneName() const {
    static const std::string none = "None";
    return m_currentScene ? m_currentScene->GetName() : none;
  }
};
