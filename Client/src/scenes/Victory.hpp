#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Helpers/EntityHelper.hpp"
#include "audio/AudioSubsystem.hpp"
#include "engine/GameEngine.hpp"
#include "network/NetworkSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"

class VictoryScene : public Scene {
 private:
  std::vector<Entity> m_entities;
  std::vector<std::tuple<uint16_t, uint32_t, uint8_t>>
      m_scores;  // playerId, score, rank
  bool m_isInitialized;

 public:
  VictoryScene() : m_isInitialized(false) { m_name = "victory"; }

  void OnEnter() override {
    std::cout << "\n=== VICTORY ===" << std::endl;

    // Récupérer les scores depuis la scène précédente
    m_scores = GetSceneData()
                   .Get<std::vector<std::tuple<uint16_t, uint32_t, uint8_t>>>(
                       "scores", {});

    // Musique de victoire
    GetAudio()->LoadMusic("victory_music", "../assets/victory_music.ogg");
    GetAudio()->PlayMusic("victory_music", 0);

    // Background
    Entity background =
        CreateSprite(GetRegistry(), "background", {400, 300}, -10);
    m_entities.push_back(background);

    // Titre Victory
    auto* titleText = GetUI()->AddElement<UIText>(250, 80, "VICTORY!", "", 60,
                                                  SDL_Color{0, 255, 0, 255});
    titleText->SetLayer(10);

    // Tableau des scores
    auto* scoresTitle = GetUI()->AddElement<UIText>(
        280, 150, "SCORES", "", 30, SDL_Color{255, 255, 0, 255});
    scoresTitle->SetLayer(10);

    int yPos = 200;
    for (const auto& [playerId, score, rank] : m_scores) {
      std::string rankStr = "#" + std::to_string(rank);
      std::string playerStr = "Player " + std::to_string(playerId);
      std::string scoreStr = std::to_string(score) + " pts";

      std::string line = rankStr + "  " + playerStr + "  -  " + scoreStr;

      // Couleur spéciale pour le premier
      SDL_Color color = (rank == 1) ? SDL_Color{255, 215, 0, 255}
                                    : SDL_Color{255, 255, 255, 255};

      auto* scoreText =
          GetUI()->AddElement<UIText>(180, yPos, line, "", 25, color);
      scoreText->SetLayer(10);

      yPos += 40;
    }

    // Si pas de scores (fallback)
    if (m_scores.empty()) {
      auto* noScores =
          GetUI()->AddElement<UIText>(250, 250, "No scores available", "", 20,
                                      SDL_Color{200, 200, 200, 255});
      noScores->SetLayer(10);
    }

    // Instructions retour
    auto* returnText =
        GetUI()->AddElement<UIText>(110, 450, "Press SPACE to return to Lobby",
                                    "", 25, SDL_Color{200, 200, 200, 255});
    returnText->SetLayer(10);

    m_isInitialized = true;

    // Debug
    std::cout << "Scores count: " << m_scores.size() << std::endl;
    for (const auto& [playerId, score, rank] : m_scores) {
      std::cout << "  Rank " << static_cast<int>(rank) << ": Player "
                << playerId << " - " << score << " pts" << std::endl;
    }
  }

  void OnExit() override {
    std::cout << "Leaving Victory screen..." << std::endl;
    for (auto& entity : m_entities) {
      if (GetRegistry().is_entity_valid(entity)) {
        GetRegistry().kill_entity(entity);
      }
    }
    m_entities.clear();
    m_scores.clear();
    GetUI()->Clear();
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {}

  void Render() override {
    if (!m_isInitialized) return;
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
