#include "scene/SceneManager.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

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

Scene::Scene(GameEngine* engine, SceneManager* sceneManager,
             const std::string& name)
    : m_engine(engine),
      m_sceneManager(sceneManager),
      m_name(name),
      m_uiManager(engine->GetRenderer(), &engine->GetTextureManager(), 800,
                  600) {}

Registry& Scene::GetRegistry() { return m_engine->GetRegistry(); }

TextureManager& Scene::GetTextures() { return m_engine->GetTextureManager(); }

AnimationManager& Scene::GetAnimations() {
  return m_engine->GetAnimationManager();
}

NetworkManager& Scene::GetNetwork() { return m_engine->GetNetworkManager(); }

SDL_Renderer* Scene::GetRenderer() { return m_engine->GetRenderer(); }
Vector2 Scene::GetCameraPosition() { return m_engine->GetCameraPosition(); }

InputManager& Scene::GetInput() { return m_engine->GetInputManager(); }

void Scene::ChangeScene(const std::string& sceneName) {
  if (m_sceneManager) {
    m_sceneManager->ChangeScene(sceneName);
  }
}

SceneData& Scene::GetSceneData() { 
  return m_sceneManager->GetSceneData(); 
}

void Scene::RenderSprites(int minLayer, int maxLayer) {
  auto& transforms = GetRegistry().get_components<Transform>();
  auto& sprites = GetRegistry().get_components<Sprite>();

  for (auto&& [transform, sprite] : Zipper(transforms, sprites)) {
    if (!sprite.visible) continue;
    if (sprite.layer >= minLayer && sprite.layer <= maxLayer) {
      SDL_Texture* texture = GetTextures().GetTexture(sprite.textureKey);
      if (!texture) continue;

      SDL_Rect sourceRect = sprite.sourceRect;
      if (sourceRect.w == 0 || sourceRect.h == 0) {
        int textureWidth, textureHeight;
        GetTextures().GetTextureSize(sprite.textureKey, textureWidth,
                                     textureHeight);
        sourceRect = {0, 0, textureWidth, textureHeight};
      }

      Vector2 camPos = GetCameraPosition();
      SDL_Rect destRect = {
          static_cast<int>(transform.position.x - camPos.x -
                           (sourceRect.w * transform.scale.x * sprite.pivot.x)),
          static_cast<int>(transform.position.y - camPos.y -
                           (sourceRect.h * transform.scale.y * sprite.pivot.y)),
          static_cast<int>(sourceRect.w * transform.scale.x),
          static_cast<int>(sourceRect.h * transform.scale.y)};

      SDL_RenderCopyEx(GetRenderer(), texture, &sourceRect, &destRect,
                       transform.rotation, nullptr, SDL_FLIP_NONE);
    }
  }
}

void Scene::RenderSpritesLayered() {
  auto& transforms = GetRegistry().get_components<Transform>();
  auto& sprites = GetRegistry().get_components<Sprite>();

  struct RenderData {
    const Transform* transform;
    const Sprite* sprite;
  };

  std::vector<RenderData> renderList;
  for (auto&& [transform, sprite] : Zipper(transforms, sprites)) {
    renderList.push_back({&transform, &sprite});
  }

  std::sort(renderList.begin(), renderList.end(),
            [](const RenderData& a, const RenderData& b) {
              return a.sprite->layer < b.sprite->layer;
            });

  for (const auto& data : renderList) {
    if (!data.sprite->visible) continue;
    SDL_Texture* texture = GetTextures().GetTexture(data.sprite->textureKey);
    if (!texture) continue;

    SDL_Rect sourceRect = data.sprite->sourceRect;
    if (sourceRect.w == 0 || sourceRect.h == 0) {
      int textureWidth, textureHeight;
      GetTextures().GetTextureSize(data.sprite->textureKey, textureWidth,
                                   textureHeight);
      sourceRect = {0, 0, textureWidth, textureHeight};
    }

    Vector2 camPos = GetCameraPosition();
    SDL_Rect destRect = {
        static_cast<int>(
            data.transform->position.x - camPos.x -
            (sourceRect.w * data.transform->scale.x * data.sprite->pivot.x)),
        static_cast<int>(
            data.transform->position.y - camPos.y -
            (sourceRect.h * data.transform->scale.y * data.sprite->pivot.y)),
        static_cast<int>(sourceRect.w * data.transform->scale.x),
        static_cast<int>(sourceRect.h * data.transform->scale.y)};

    SDL_RenderCopyEx(GetRenderer(), texture, &sourceRect, &destRect,
                     data.transform->rotation, nullptr, SDL_FLIP_NONE);
  }
}

void Scene::QuitGame() {
  if (m_engine) {
    m_engine->Stop();
  }
}
