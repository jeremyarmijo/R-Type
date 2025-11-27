#include "scene/SceneManager.hpp"

#include <iostream>
#include <string>

#include "engine/GameEngine.hpp"

void SceneManager::ChangeScene(const std::string& name) {
  auto it = m_scenes.find(name);
  if (it != m_scenes.end()) {
    m_nextScene = it->second.get();
    std::cout << "Scene change requested: " << name << std::endl;
  } else {
    std::cerr << "ERROR: Scene '" << name << "' not found!" << std::endl;
    std::cerr << "Available scenes: ";
    for (const auto& pair : m_scenes) {
      std::cerr << pair.first << " ";
    }
    std::cerr << std::endl;
  }
}

void SceneManager::Update(float deltaTime) {
  if (m_nextScene && m_nextScene != m_currentScene) {
    std::cout << "\n=== SCENE TRANSITION ===" << std::endl;

    try {
      if (m_currentScene) {
        std::cout << "Exiting scene: " << m_currentScene->GetName()
                  << std::endl;
        m_currentScene->OnExit();
      }

      if (m_registry) {
        std::cout << "Clearing all entities from registry..." << std::endl;
        m_registry->clear_all_entities();

        SDL_Delay(16);
      }

      m_currentScene = m_nextScene;
      std::cout << "Entering scene: " << m_currentScene->GetName() << std::endl;
      m_currentScene->OnEnter();
      m_nextScene = nullptr;

      std::cout << "Scene transition complete" << std::endl;

      if (m_registry) {
        m_registry->print_debug_info();
      }
    } catch (const std::exception& e) {
      std::cerr << "ERROR during scene transition: " << e.what() << std::endl;
      m_nextScene = nullptr;
    }

    std::cout << "========================\n" << std::endl;
  }

  if (m_currentScene) {
    try {
      m_currentScene->Update(deltaTime);
    } catch (const std::exception& e) {
      std::cerr << "ERROR in scene update: " << e.what() << std::endl;
    }
  }
}

void SceneManager::Render() {
  if (m_currentScene) {
    try {
      m_currentScene->Render();
    } catch (const std::exception& e) {
      std::cerr << "ERROR in scene render: " << e.what() << std::endl;
    }
  }
}

void SceneManager::HandleEvent(SDL_Event& event) {
  if (m_currentScene) {
    try {
      m_currentScene->HandleEvent(event);
    } catch (const std::exception& e) {
      std::cerr << "ERROR in scene event handling: " << e.what() << std::endl;
    }
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
