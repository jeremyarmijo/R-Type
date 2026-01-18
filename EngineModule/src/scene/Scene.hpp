#pragma once
#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>

#include "ecs/Entity.hpp"

class GameEngine;
class SceneManager;
class SceneData;
class Registry;
class RenderingSubsystem;
class AudioSubsystem;
class InputSubsystem;
class NetworkSubsystem;
class UIManager;

class Scene {
 protected:
  GameEngine* m_engine;
  SceneManager* m_sceneManager;
  std::string m_name;

 public:
  Scene() : m_engine(nullptr) {}
  virtual ~Scene() = default;

  void SetEngine(GameEngine* engine) { m_engine = engine; }
  void SetSceneManager(SceneManager* manager) { m_sceneManager = manager; }

  virtual void OnEnter() = 0;
  virtual void OnExit() = 0;
  virtual void Update(float deltaTime) = 0;
  virtual void Render() = 0;
  virtual void HandleEvent(SDL_Event& event) {}

  virtual std::unordered_map<uint16_t, Entity> GetPlayers() = 0;

  const std::string& GetName() const { return m_name; }

 protected:
  Registry& GetRegistry();
  RenderingSubsystem* GetRendering();
  AudioSubsystem* GetAudio();
  InputSubsystem* GetInput();
  UIManager* GetUI();
  NetworkSubsystem* GetNetwork();

  SceneData& GetSceneData();
  void ChangeScene(const std::string& sceneName);
  void QuitGame();
};
