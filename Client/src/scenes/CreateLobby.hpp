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

class CreateLobby : public Scene {
 private:
  bool m_isInitialized;
  bool m_settingsTransition;
  std::vector<Entity> m_entities;

  UIButton* m_publicLobbyButton;
  UIButton* m_privateLobbyButton;
  UITextInput* m_lobbyNameInput;
  UITextInput* m_lobbypasswordInput;
  UIButton* m_difficulty1Button;
  UIButton* m_difficulty2Button;
  UIButton* m_difficulty3Button;
  UIButton* m_difficulty4Button;
  UIButton* m_difficulty5Button;
  UIButton* m_saveButton;

  UIButton* m_backButton;

  uint8_t difficulty = 1;
  bool isPrivate = false;
  std::string password = "";

 public:
  CreateLobby(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "createLobby"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_privateLobbyButton(nullptr),
        m_publicLobbyButton(nullptr),
        m_difficulty1Button(nullptr),
        m_difficulty2Button(nullptr),
        m_difficulty3Button(nullptr),
        m_difficulty4Button(nullptr),
        m_difficulty5Button(nullptr),
        m_lobbyNameInput(nullptr),
        m_lobbypasswordInput(nullptr),
        m_saveButton(nullptr),
        m_backButton(nullptr) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING CREATE LOBBY MENU SCENE ===" << std::endl;

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

      m_difficulty1Button = GetUI().AddElement<UIButton>(275, 400, 50, 35, "1");
      m_difficulty1Button->SetLayer(9);
      m_difficulty1Button->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        difficulty = 1;
        std::cout << "LOBBY Difficulty = (" << static_cast<int>(difficulty)
                  << ")" << std::endl;
        m_difficulty1Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_difficulty2Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty3Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty4Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty5Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
      });
      m_difficulty1Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                     {10, 20, 60, 220});

      m_difficulty2Button = GetUI().AddElement<UIButton>(325, 400, 50, 35, "2");
      m_difficulty2Button->SetLayer(9);
      m_difficulty2Button->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        difficulty = 2;
        std::cout << "LOBBY Difficulty = (" << static_cast<int>(difficulty)
                  << ")" << std::endl;
        m_difficulty2Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_difficulty1Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty3Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty4Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty5Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
      });
      m_difficulty2Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});

      m_difficulty3Button = GetUI().AddElement<UIButton>(375, 400, 50, 35, "3");
      m_difficulty3Button->SetLayer(9);
      m_difficulty3Button->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        difficulty = 3;
        std::cout << "LOBBY Difficulty = (" << static_cast<int>(difficulty)
                  << ")" << std::endl;
        m_difficulty3Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_difficulty1Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty2Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty4Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty5Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
      });
      m_difficulty3Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});

      m_difficulty4Button = GetUI().AddElement<UIButton>(425, 400, 50, 35, "4");
      m_difficulty4Button->SetLayer(9);
      m_difficulty4Button->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        difficulty = 4;
        std::cout << "LOBBY Difficulty = (" << static_cast<int>(difficulty)
                  << ")" << std::endl;
        m_difficulty4Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_difficulty1Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty2Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty3Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty5Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
      });
      m_difficulty4Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});

      m_difficulty5Button = GetUI().AddElement<UIButton>(475, 400, 50, 35, "5");
      m_difficulty5Button->SetLayer(9);
      m_difficulty5Button->SetOnClick([this]() {
        std::cout
            << "JOIN_LOBBY button clicked - transitioning to join screen..."
            << std::endl;
        GetAudio().PlaySound("button");
        difficulty = 5;
        std::cout << "LOBBY Difficulty = (" << static_cast<int>(difficulty)
                  << ")" << std::endl;
        m_difficulty5Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_difficulty1Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty2Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty3Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_difficulty4Button->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
      });
      m_difficulty5Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});

      m_publicLobbyButton =
          GetUI().AddElement<UIButton>(550, 100, 120, 40, "PUBLIC");
      m_publicLobbyButton->SetLayer(9);
      m_publicLobbyButton->SetOnClick([this]() {
        std::cout << "Options button clicked!" << std::endl;
        GetAudio().PlaySound("button");

        // ChangeScene("options");
        m_privateLobbyButton->SetColors(
            {100, 100, 100, 255}, {150, 150, 150, 255}, {80, 80, 80, 255});
        m_publicLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                       {10, 20, 60, 220});
        m_lobbypasswordInput->SetVisible(false);
        isPrivate = false;
      });
      m_publicLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                     {10, 20, 60, 220});

      m_privateLobbyButton =
          GetUI().AddElement<UIButton>(660, 100, 120, 40, "PRIVATE");
      m_privateLobbyButton->SetLayer(9);
      m_privateLobbyButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");

        // ChangeScene("options");
        m_publicLobbyButton->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_privateLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                        {10, 20, 60, 220});
        m_lobbypasswordInput->SetVisible(true);
        isPrivate = true;
      });
      m_privateLobbyButton->SetColors({100, 100, 100, 255},
                                      {150, 150, 150, 255}, {80, 80, 80, 255});

      m_saveButton = GetUI().AddElement<UIButton>(350, 500, 120, 40, "SAVE");
      m_saveButton->SetLayer(9);
      m_saveButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");

        // ChangeScene("options");
        std::string lobbyName = m_lobbyNameInput->GetText();
        std::string passwordInput = m_lobbypasswordInput->GetText();

        if (lobbyName.empty()) {
          std::cout << "ERROR: Please enter a username!" << std::endl;
          m_lobbyNameInput->Focus();
          return;
        }
        if (isPrivate && passwordInput.empty()) {
          std::cout << "ERROR: Please enter a password!" << std::endl;
          m_lobbyNameInput->Focus();
          return;
        }
        std::string playerName =
            GetSceneData().Get<std::string>("playerName", "");
        Action lobbyCreate{
            ActionType::LOBBY_CREATE,
            LobbyCreate{lobbyName, playerName, passwordInput, 5, difficulty}};
        GetNetwork().SendAction(lobbyCreate);
        std::cout << "ACTION LOBBY_CREATE SEND:" << std::endl;
        std::cout << "NAME LOBBY: (" << lobbyName << ")" << std::endl;
        std::cout << "PASSWORD: (" << passwordInput << ")" << std::endl;
        std::cout << "DIFFICULTY: (" << static_cast<int>(difficulty) << ")"
                  << std::endl;
      });
      m_saveButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                              {80, 80, 80, 255});

      m_lobbyNameInput =
          GetUI().AddElement<UITextInput>(250, 200, 300, 50, "Lobby Name");
      m_lobbyNameInput->SetMaxLength(50);
      m_lobbyNameInput->SetText("");
      m_lobbyNameInput->SetTextColor({255, 255, 255, 255});
      m_lobbyNameInput->SetBackgroundColor({40, 40, 50, 255});
      m_lobbyNameInput->SetBorderColor({100, 100, 120, 255},
                                       {100, 150, 255, 255});
      m_lobbyNameInput->SetLayer(9);
      m_lobbyNameInput->SetVisible(true);

      m_lobbypasswordInput =
          GetUI().AddElement<UITextInput>(250, 300, 300, 50, "Lobby Password");
      m_lobbypasswordInput->SetMaxLength(50);
      m_lobbypasswordInput->SetText("");
      m_lobbypasswordInput->SetTextColor({255, 255, 255, 255});
      m_lobbypasswordInput->SetBackgroundColor({40, 40, 50, 255});
      m_lobbypasswordInput->SetBorderColor({100, 100, 120, 255},
                                           {100, 150, 255, 255});
      m_lobbypasswordInput->SetLayer(9);
      m_lobbypasswordInput->SetVisible(false);

      std::cout << "=================================\n" << std::endl;

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }
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

    Event e = GetNetwork().PopEvent();
    if (e.type == EventType::LOBBY_JOIN_RESPONSE) {
      const auto* data = std::get_if<LOBBY_JOIN_RESPONSE>(&e.data);
      if (data->success == 0) return;
      GetSceneData().Set("playerId", data->playerId);
      ChangeScene("lobbyInfoPlayer");
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
