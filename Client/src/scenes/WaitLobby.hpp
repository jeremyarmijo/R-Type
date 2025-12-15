#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIImage.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"
#include "ui/UITextInput.hpp"

class WaitLobby : public Scene {
 private:
  bool m_isInitialized;
  std::vector<Entity> m_entities;

 public:
  WaitLobby(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "wait"), m_isInitialized(false) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING WAITING SCENE ===" << std::endl;

    try {
      m_isInitialized = false;

      TextureManager& textures = GetTextures();

      std::cout << "Loading textures..." << std::endl;
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }
      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);

      std::cout << "Creating UI Elements..." << std::endl;
      auto* text =
          GetUI().AddElement<UIText>(70, 250, "Waiting for game start...", "",
                                     50, SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }

    std::cout << "=================================\n" << std::endl;
  }

  void OnExit() override {
    std::cout << "\n=== EXITING WAIT SCENE ===" << std::endl;

    m_entities.clear();
    m_isInitialized = false;

    std::cout << "wait cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    Event e = GetNetwork().PopEvent();
    if (e.type == EventType::GAME_START) {
      const auto* data = std::get_if<GAME_START>(&e.data);
      GetSceneData().Set("posX", data->playerSpawnX);
      GetSceneData().Set("posY", data->playerSpawnY);
      ChangeScene("game");
    }
  }

  void Render() override {
    if (!m_isInitialized) return;

    RenderSpritesLayered();
    GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (GetUI().HandleEvent(event)) {
      return;
    }
  }
};
