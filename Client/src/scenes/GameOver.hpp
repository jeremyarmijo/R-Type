#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>

#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "audio/AudioSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "Helpers/EntityHelper.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"

class GameOverScene : public Scene {
 private:
  std::vector<Entity> m_entities;
  int m_finalScore;
  bool m_isInitialized;

 public:
  GameOverScene()
      : m_finalScore(0),
        m_isInitialized(false) { m_name = "GameOver"; }

  void SetScore(int score) { m_finalScore = score; }

  void OnEnter() override {
    std::cout << "\n=== GAME OVER ===" << std::endl;

    GetAudio()->LoadMusic("gameover_music",
                         "../assets/gameover_music.ogg");
    GetAudio()->PlayMusic("gameover_music", 0);

    Entity background = CreateSprite(GetRegistry(), "background", {400, 300}, -10);
    m_entities.push_back(background);
    auto* text = GetUI()->AddElement<UIText>(220, 240, "GAME OVER...", "", 50,
                                            SDL_Color{255, 255, 255, 255});
    auto* returnText = GetUI()->AddElement<UIText>(
        110, 330, "press \"space\" to return to Lobby", "", 30,
        SDL_Color{255, 255, 255, 255});
    text->SetVisible(true);
    text->SetLayer(10);

    m_isInitialized = true;
  }

  void OnExit() override {
    std::cout << "Leaving Game Over screen..." << std::endl;
    GetUI()->Clear();
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {}

  void Render() override {
    if (!m_isInitialized) return;

    // RenderSpritesLayered();
    // GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_SPACE ||
          event.key.keysym.sym == SDLK_RETURN) {
        std::cout << "Returning to Lobby..." << std::endl;
        ChangeScene("lobbyInfoPlayer");
      }
    }
  }

  std::unordered_map<uint16_t, Entity> GetPlayers() override {
    return std::unordered_map<uint16_t, Entity>(); 
  }
};

extern "C" {
    Scene* CreateScene();
}
