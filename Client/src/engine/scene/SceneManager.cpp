#include "scene/SceneManager.hpp"

#include <string>

#include "engine/GameEngine.hpp"

void SceneManager::ChangeScene(const std::string& name) {
  auto it = m_scenes.find(name);
  if (it != m_scenes.end()) {
    m_nextScene = it->second.get();
  }
}

void SceneManager::Update(float deltaTime) {
  if (m_nextScene && m_nextScene != m_currentScene) {
    if (m_currentScene) {
      m_currentScene->OnExit();
    }
    m_currentScene = m_nextScene;
    m_currentScene->OnEnter();
    m_nextScene = nullptr;
  }

  if (m_currentScene) {
    m_currentScene->Update(deltaTime);
  }
}

void SceneManager::Render() {
  if (m_currentScene) {
    m_currentScene->Render();
  }
}

void SceneManager::HandleEvent(SDL_Event& event) {
  if (m_currentScene) {
    m_currentScene->HandleEvent(event);
  }
}

Registry& Scene::GetRegistry() { return m_engine->GetRegistry(); }

TextureManager& Scene::GetTextures() { return m_engine->GetTextureManager(); }

AnimationManager& Scene::GetAnimations() {
  return m_engine->GetAnimationManager();
}

InputManager& Scene::GetInput() { return m_engine->GetInputManager(); }

void Scene::ChangeScene(const std::string& sceneName) {
  if (m_sceneManager) {
    m_sceneManager->ChangeScene(sceneName);
  }
}
