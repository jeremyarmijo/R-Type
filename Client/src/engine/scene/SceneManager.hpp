#pragma once
#include <SDL2/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ecs/Registry.hpp"
#include "graphics/AnimationManager.hpp"
#include "graphics/TextureManager.hpp"
#include "inputs/InputManager.hpp"

class GameEngine;
class SceneManager;

class Scene {
 protected:
  GameEngine* m_engine;
  SceneManager* m_sceneManager;
  std::string m_name;

 public:
  Scene(GameEngine* engine, SceneManager* sceneManager, const std::string& name)
      : m_engine(engine), m_sceneManager(sceneManager), m_name(name) {}

  virtual ~Scene() {}
  virtual void OnEnter() = 0;
  virtual void OnExit() = 0;
  virtual void Update(float deltaTime) {}
  virtual void Render() {}
  virtual void HandleEvent(SDL_Event& event) {}
  const std::string& GetName() const { return m_name; }

 protected:
  Registry& GetRegistry();
  TextureManager& GetTextures();
  AnimationManager& GetAnimations();
  InputManager& GetInput();

  void ChangeScene(const std::string& sceneName);
};

class SceneManager {
 private:
  std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
  Scene* m_currentScene;
  Scene* m_nextScene;

 public:
  SceneManager() : m_currentScene(nullptr), m_nextScene(nullptr) {}

  template <typename T, typename... Args>
  void RegisterScene(const std::string& name, Args&&... args) {
    m_scenes[name] = std::make_unique<T>(std::forward<Args>(args)...);
  }

  void ChangeScene(const std::string& name);
  void Update(float deltaTime);
  void Render();
  void HandleEvent(SDL_Event& event);
  Scene* GetCurrentScene() { return m_currentScene; }
};
