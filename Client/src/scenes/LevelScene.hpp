#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Helpers/EntityHelper.hpp"
#include "audio/AudioSubsystem.hpp"
#include "engine/GameEngine.hpp"
#include "network/NetworkSubsystem.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIImage.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"
#include "ui/UITextInput.hpp"

class Level : public Scene {
 private:
  bool m_isInitialized;
  float m_timer;
  float m_duration;
  std::string m_nextLevel;

  std::vector<Entity> m_entities;

 public:
  explicit Level(const std::string& nextLevel)
      : m_isInitialized(false),
        m_timer(0.f),
        m_duration(2.f),  // durée de l'écran de transition
        m_nextLevel(nextLevel) {
    m_name = "levelTransition";
  }
  void OnEnter() override {
    std::cout << "\n=== ENTERING LEVEL TRANSITION SCENE ===" << std::endl;

    try {
      m_isInitialized = false;
      m_timer = 0.f;  // Reset le timer !

      uint8_t levelNum = GetSceneData().Get<uint8_t>("nextLevel", 1);
      m_nextLevel = "game";  // Retourne toujours au jeu après

      // Fond
      Entity background =
          CreateSprite(GetRegistry(), "background", {400, 300}, -10);
      m_entities.push_back(background);

      auto* text = GetUI()->AddElement<UIText>(
          300, 250, "Level " + std::to_string(levelNum), "", 50,
          SDL_Color{255, 255, 255, 255});
      text->SetLayer(10);
      m_isInitialized = true;
      std::cout << "Transition to Level " << static_cast<int>(levelNum)
                << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }
  }

  void OnExit() override {
    std::cout << "\n=== EXITING LEVEL TRANSITION SCENE ===" << std::endl;
    m_entities.clear();
    GetUI()->Clear();
    m_isInitialized = false;
    std::cout << "Transition cleanup complete" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    m_timer += deltaTime;
    if (m_timer >= m_duration) {
      ChangeScene(m_nextLevel);  // Passe au niveau suivant
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    // RenderSpritesLayered();
    GetUI()->Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (GetUI()->HandleEvent(event)) return;
  }

  std::unordered_map<uint16_t, Entity> GetPlayers() override {
    return std::unordered_map<uint16_t, Entity>();
  }
};

extern "C" {
Scene* CreateScene();
}
