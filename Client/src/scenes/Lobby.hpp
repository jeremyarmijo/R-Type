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

class LobbyMenu : public Scene {
 private:
  bool m_isInitialized;
  bool m_settingsTransition;
  std::vector<Entity> m_entities;

  UIButton* m_joinLobbyButton;
  UIButton* m_createLobbyButton;
  UIButton* m_backButton;

 public:
  LobbyMenu(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "lobby"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_joinLobbyButton(nullptr),
        m_createLobbyButton(nullptr),
        m_backButton(nullptr) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING LOBBY MENU SCENE ===" << std::endl;

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

      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);

      std::cout << "Creating UI Elements..." << std::endl;
      auto* text = GetUI().AddElement<UIText>(50, 40, "R-Type", "", 50,
                                              SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_joinLobbyButton =
          GetUI().AddElement<UIButton>(250, 350, 270, 70, "JOIN LOBBY");
      m_joinLobbyButton->SetLayer(9);
      m_joinLobbyButton->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        ChangeScene("lobbyjoin");
      });
      m_joinLobbyButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});

      m_createLobbyButton =
          GetUI().AddElement<UIButton>(250, 200, 270, 70, "CREATE LOBBY");
      m_createLobbyButton->SetLayer(9);
      m_createLobbyButton->SetOnClick([this]() {
        std::cout << "Options button clicked!" << std::endl;
        GetAudio().PlaySound("button");

        /*Action lobbyCreate{ActionType::LOGIN_REQUEST,
                            LoginReq{username, "hashedpassword"}};
        GetNetwork().SendAction(lobbyCreate);*/
        ChangeScene("createLobby");
      });
      m_createLobbyButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});

      m_backButton = GetUI().AddElement<UIButton>(250, 500, 270, 70, "Back");
      m_backButton->SetLayer(9);
      m_backButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        std::cout << "Back to main menu..." << std::endl;
        ChangeScene("join");
      });
      m_backButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                              {130, 30, 30, 255});
      m_backButton->SetVisible(true);

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }

    std::cout << "=================================\n" << std::endl;
  }

  void OnExit() override {
    std::cout << "\n=== EXITING LOBBY SCENE ===" << std::endl;

    m_entities.clear();
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

    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_r) {
        std::cout << "Restarting level..." << std::endl;
        ChangeScene("game");
      } else if (event.key.keysym.sym == SDLK_x) {
        std::cout << "Quitting to Game Over screen..." << std::endl;
        ChangeScene("gameover");
      }
    }
  }
};
