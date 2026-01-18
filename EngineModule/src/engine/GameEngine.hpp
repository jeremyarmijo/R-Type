#pragma once
#include <SDL2/SDL.h>
#include <dlfcn.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "dynamicLibLoader/DLLoader.hpp"
#include "ecs/Registry.hpp"
#include "engine/ISubsystem.hpp"
#include "scene/SceneManager.hpp"

struct SubsystemPlugin {
  std::unique_ptr<DLLoader<ISubsystem>> loader;
  ISubsystem* instance;
};

class GameEngine {
 private:
  Registry m_registry;

  std::unique_ptr<SceneManager> m_sceneManager;

  std::unordered_map<SubsystemType, SubsystemPlugin> m_subsystems;
  std::vector<SubsystemType> m_updateOrder;

  bool m_running;
  float m_deltaTime;
  int m_windowWidth;
  int m_windowHeight;

 public:
  GameEngine();
  ~GameEngine();

  bool Initialize(const std::string& title, int width, int height);
  void Shutdown();

  bool LoadSubsystem(SubsystemType type, const std::string& pluginPath);
  void UnloadSubsystem(SubsystemType type);
  ISubsystem* GetSubsystem(SubsystemType type);
  bool HasSubsystem(SubsystemType type) const;

  bool LoadSceneModule(const std::string& name, const std::string& soPath) {
    return m_sceneManager->LoadSceneModule(name, soPath);
  }

  void ChangeScene(const std::string& name) {
    m_sceneManager->ChangeScene(name);
  }

  Scene* GetCurrentScene() { return m_sceneManager->GetCurrentScene(); }

  SceneData& GetSceneData() { return m_sceneManager->GetSceneData(); }

  void Run();
  void Stop() { m_running = false; }

  Registry& GetRegistry() { return m_registry; }
  SceneManager& GetSceneManager() { return *m_sceneManager; }

  int GetWindowWidth() const { return m_windowWidth; }
  int GetWindowHeight() const { return m_windowHeight; }
  float GetDeltaTime() const { return m_deltaTime; }
  void Update(float deltaTime);

 private:
  void HandleEvents();
};
