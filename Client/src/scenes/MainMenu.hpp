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

class MainMenu : public Scene {
 private:
  bool m_isInitialized;
  bool m_settingsTransition;
  std::vector<Entity> m_entities;

  UIButton* m_playButton;
  UIButton* m_settingsButton;
  UIButton* m_quitButton;
  UITextInput* m_usernameInput;
  UITextInput* m_serverInput;
  UIButton* m_joinButton;
  UIButton* m_backButton;

 public:
  MainMenu(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "menu"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_playButton(nullptr),
        m_settingsButton(nullptr),
        m_quitButton(nullptr),
        m_usernameInput(nullptr),
        m_serverInput(nullptr),
        m_joinButton(nullptr),
        m_backButton(nullptr) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING MAIN MENU SCENE ===" << std::endl;

    try {
      m_isInitialized = false;

      TextureManager& textures = GetTextures();
      AnimationManager& animations = GetAnimations();
      AudioManager& audio = GetAudio();

      audio.LoadMusic("menu_music", "../Client/assets/menu_music.ogg");
      audio.LoadMusic("game_music", "../Client/assets/rtype_music.ogg");
      audio.LoadSound("explosion", "../Client/assets/explosion.wav");
      audio.LoadSound("button", "../Client/assets/button.wav");
      audio.LoadSound("shoot", "../Client/assets/shoot.wav");
      audio.LoadSound("hitmarker", "../Client/assets/hitmarker.wav");

      audio.PlayMusic("menu_music");

      std::cout << "Loading textures..." << std::endl;
      if (!textures.GetTexture("player")) {
        textures.LoadTexture("player", "../Client/assets/player.png");
      }
      if (!textures.GetTexture("boss")) {
        textures.LoadTexture("boss", "../Client/assets/boss1.png");
      }
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }

      std::cout << "Creating animations..." << std::endl;
      animations.CreateAnimation("boss", "boss",
                                 {{{27, 2, 154, 203}, 0.6f},
                                  {{189, 2, 154, 203}, 0.5f},
                                  {{351, 2, 154, 203}, 0.6f},
                                  {{189, 2, 154, 203}, 0.5f}},
                                 true);
      animations.CreateAnimation("bosschild", "boss",
                                 {{{570, 1964, 31, 31}, 0.5f},
                                  {{603, 1964, 31, 31}, 0.6f},
                                  {{636, 1964, 31, 31}, 0.5f},
                                  {{603, 1964, 31, 31}, 0.6f}},
                                 true);
      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);
      std::cout << "Creating animated sprite..." << std::endl;
      Entity boss = m_engine->CreateAnimatedSprite("boss", {600, 300}, "boss");
      auto& playerTransform =
          m_engine->GetRegistry().get_components<Transform>()[boss];
      if (playerTransform) playerTransform->scale = {2.0f, 2.0f};
      m_entities.push_back(boss);
      Entity bossChild =
          m_engine->CreateAnimatedSprite("boss", {622, 317}, "bosschild");
      auto& bosschild =
          m_engine->GetRegistry().get_components<Transform>()[bossChild];
      if (bosschild) bosschild->scale = {2.1f, 2.1f};
      m_entities.push_back(bossChild);

      std::cout << "Creating Player animations..." << std::endl;
      createPlayerAnimations();

      std::cout << "Creating UI Elements..." << std::endl;
      auto* text = GetUI().AddElement<UIText>(50, 40, "R-Type", "", 50,
                                              SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_playButton =
          GetUI().AddElement<UIButton>(70, 125, 200, 50, "Play Game");
      m_playButton->SetLayer(9);
      m_playButton->SetOnClick([this]() {
        std::cout << "Play button clicked - transitioning to join screen..."
                  << std::endl;
        GetAudio().PlaySound("button");
        ChangeScene("join");
      });
      m_playButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                              {80, 80, 80, 255});

      m_settingsButton =
          GetUI().AddElement<UIButton>(70, 200, 200, 50, "Options");
      m_settingsButton->SetLayer(9);
      m_settingsButton->SetOnClick([this]() {
        std::cout << "Options button clicked!" << std::endl;
        GetAudio().PlaySound("button");
        ChangeScene("options");
      });
      m_settingsButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                  {80, 80, 80, 255});

      m_quitButton = GetUI().AddElement<UIButton>(70, 275, 200, 50, "Quit");
      m_quitButton->SetLayer(9);
      m_quitButton->SetOnClick([this]() { QuitGame(); });
      m_quitButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                              {130, 30, 30, 255});

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }

    std::cout << "=================================\n" << std::endl;
  }

  void createPlayerAnimations() {
    AnimationManager& animations = GetAnimations();
    animations.CreateAnimation("blue_player", "player",
                               {{{1, 3, 31, 13}, 0.3f},
                                {{34, 3, 31, 13}, 0.3f},
                                {{68, 3, 31, 13}, 0.3f},
                                {{100, 3, 31, 13}, 0.3f},
                                {{133, 3, 31, 13}, 0.3f},
                                {{100, 3, 31, 13}, 0.3f},
                                {{68, 3, 31, 13}, 0.3f},
                                {{34, 3, 31, 13}, 0.3f}},
                               true);

    animations.CreateAnimation("pink_player", "player",
                               {{{1, 20, 31, 13}, 0.3f},
                                {{34, 20, 31, 13}, 0.3f},
                                {{68, 20, 31, 13}, 0.3f},
                                {{100, 20, 31, 13}, 0.3f},
                                {{133, 20, 31, 13}, 0.3f},
                                {{100, 20, 31, 13}, 0.3f},
                                {{68, 20, 31, 13}, 0.3f},
                                {{34, 20, 31, 13}, 0.3f}},
                               true);

    animations.CreateAnimation("green_player", "player",
                               {{{1, 37, 31, 13}, 0.3f},
                                {{34, 37, 31, 13}, 0.3f},
                                {{68, 37, 31, 13}, 0.3f},
                                {{100, 37, 31, 13}, 0.3f},
                                {{133, 37, 31, 13}, 0.3f},
                                {{100, 37, 31, 13}, 0.3f},
                                {{68, 37, 31, 13}, 0.3f},
                                {{34, 37, 31, 13}, 0.3f}},
                               true);

    animations.CreateAnimation("red_player", "player",
                               {{{1, 54, 31, 13}, 0.3f},
                                {{34, 54, 31, 13}, 0.3f},
                                {{68, 54, 31, 13}, 0.3f},
                                {{100, 54, 31, 13}, 0.3f},
                                {{133, 54, 31, 13}, 0.3f},
                                {{100, 54, 31, 13}, 0.3f},
                                {{68, 54, 31, 13}, 0.3f},
                                {{34, 54, 31, 13}, 0.3f}},
                               true);

    animations.CreateAnimation("darkblue_player", "player",
                               {{{1, 71, 31, 13}, 0.3f},
                                {{34, 71, 31, 13}, 0.3f},
                                {{68, 71, 31, 13}, 0.3f},
                                {{100, 71, 31, 13}, 0.3f},
                                {{133, 71, 31, 13}, 0.3f},
                                {{100, 71, 31, 13}, 0.3f},
                                {{68, 71, 31, 13}, 0.3f},
                                {{34, 71, 31, 13}, 0.3f}},
                               true);
  }
  void OnExit() override {
    std::cout << "\n=== EXITING MENU SCENE ===" << std::endl;

    m_entities.clear();
    GetUI().Clear();
    m_isInitialized = false;

    std::cout << "Menu cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;
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
