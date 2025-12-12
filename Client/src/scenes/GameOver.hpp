#pragma once
#include "scene/SceneManager.hpp"
#include "ui/UIManager.hpp"
#include <SDL2/SDL.h>

class GameOverScene : public Scene {
 private:
  int m_finalScore;
  bool m_isInitialized;

 public:
  GameOverScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "GameOver"),
        m_finalScore(0),
        m_isInitialized(false) {}

  void SetScore(int score) { m_finalScore = score; }

  void OnEnter() override {
    std::cout << "\n=== GAME OVER ===" << std::endl;

    m_isInitialized = true;
  }

  void OnExit() override {
    std::cout << "Leaving Game Over screen..." << std::endl;
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {}

  void Render() override {}

  void HandleEvent(SDL_Event& event) override {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_SPACE ||
          event.key.keysym.sym == SDLK_RETURN) {
        std::cout << "Restarting game..." << std::endl;
        ChangeScene("game");
      }
    }
  }
};
