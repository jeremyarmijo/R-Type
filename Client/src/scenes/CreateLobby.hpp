#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "audio/AudioSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "network/NetworkSubsystem.hpp"
#include "Helpers/EntityHelper.hpp"
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

  UIButton* m_player1Button;
  UIButton* m_player2Button;
  UIButton* m_player3Button;
  UIButton* m_player4Button;

  UIButton* m_saveButton;
  UIButton* m_backButton;

  uint8_t difficulty = 1;
  uint8_t maxPlayers = 4;
  bool isPrivate = false;
  std::string password = "";

 public:
  CreateLobby()
      : m_isInitialized(false),
        m_settingsTransition(false),
        m_privateLobbyButton(nullptr),
        m_publicLobbyButton(nullptr),
        m_difficulty1Button(nullptr),
        m_difficulty2Button(nullptr),
        m_difficulty3Button(nullptr),
        m_difficulty4Button(nullptr),
        m_difficulty5Button(nullptr),
        m_player1Button(nullptr),
        m_player2Button(nullptr),
        m_player3Button(nullptr),
        m_player4Button(nullptr),
        m_lobbyNameInput(nullptr),
        m_lobbypasswordInput(nullptr),
        m_saveButton(nullptr),
        m_backButton(nullptr) { m_name = "createLobby"; }

  void OnEnter() override {
    std::cout << "\n=== ENTERING CREATE LOBBY MENU SCENE ===" << std::endl;

    try {
      m_isInitialized = false;

      GetAudio()->LoadMusic("menu_music", "../assets/menu_music.ogg");
      GetAudio()->LoadMusic("game_music", "../assets/rtype_music.ogg");
      GetAudio()->LoadSound("explosion", "../assets/explosion.wav");
      GetAudio()->LoadSound("button", "../assets/button.wav");
      GetAudio()->LoadSound("shoot", "../assets/shoot.wav");
      GetAudio()->LoadSound("hitmarker", "../assets/hitmarker.wav");

      GetAudio()->PlayMusic("menu_music");

      if (!GetRendering()->GetTexture("background")) {
        GetRendering()->LoadTexture("background", "../assets/bg.jpg");
      }

      Entity background = CreateSprite(GetRegistry(), "background", {400, 300}, -10);
      m_entities.push_back(background);

      auto* text = GetUI()->AddElement<UIText>(50, 40, "R-Type", "", 50,
                                              SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_backButton = GetUI()->AddElement<UIButton>(50, 100, 100, 40, "BACK");
      m_backButton->SetLayer(9);
      m_backButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ChangeScene("lobby");
      });
      m_backButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                              {130, 30, 30, 255});

      auto* textPlayers = GetUI()->AddElement<UIText>(
          200, 300, "Max Players :", "", 20, SDL_Color{255, 255, 255, 255});
      textPlayers->SetVisible(true);

      m_player1Button = GetUI()->AddElement<UIButton>(275, 340, 50, 35, "1");
      m_player1Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        maxPlayers = 1;
        m_player1Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                   {10, 20, 60, 220});
        m_player2Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player3Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player4Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
      });

      m_player2Button = GetUI()->AddElement<UIButton>(325, 340, 50, 35, "2");
      m_player2Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        maxPlayers = 2;
        m_player2Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                   {10, 20, 60, 220});
        m_player1Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player3Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player4Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
      });

      m_player3Button = GetUI()->AddElement<UIButton>(375, 340, 50, 35, "3");
      m_player3Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        maxPlayers = 3;
        m_player3Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                   {10, 20, 60, 220});
        m_player1Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player2Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player4Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
      });

      m_player4Button = GetUI()->AddElement<UIButton>(425, 340, 50, 35, "4");
      m_player4Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        maxPlayers = 4;
        m_player4Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                   {10, 20, 60, 220});
        m_player1Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player2Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
        m_player3Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
      });
      m_player4Button->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                 {10, 20, 60, 220});
      m_player1Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                 {80, 80, 80, 255});
      m_player2Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                 {80, 80, 80, 255});
      m_player3Button->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                 {80, 80, 80, 255});

      auto* text2 = GetUI()->AddElement<UIText>(200, 400, "Difficulty :", "", 20,
                                               SDL_Color{255, 255, 255, 255});
      text2->SetVisible(true);

      m_difficulty1Button = GetUI()->AddElement<UIButton>(275, 440, 50, 35, "1");
      m_difficulty1Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        difficulty = 1;
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

      m_difficulty2Button = GetUI()->AddElement<UIButton>(325, 440, 50, 35, "2");
      m_difficulty2Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        difficulty = 2;
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

      m_difficulty3Button = GetUI()->AddElement<UIButton>(375, 440, 50, 35, "3");
      m_difficulty3Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        difficulty = 3;
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

      m_difficulty4Button = GetUI()->AddElement<UIButton>(425, 440, 50, 35, "4");
      m_difficulty4Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        difficulty = 4;
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

      m_difficulty5Button = GetUI()->AddElement<UIButton>(475, 440, 50, 35, "5");
      m_difficulty5Button->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        difficulty = 5;
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
          GetUI()->AddElement<UIButton>(550, 100, 100, 30, "PUBLIC");
      m_publicLobbyButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
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
          GetUI()->AddElement<UIButton>(650, 100, 102, 30, "PRIVATE");
      m_privateLobbyButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        m_publicLobbyButton->SetColors({100, 100, 100, 255},
                                       {150, 150, 150, 255}, {80, 80, 80, 255});
        m_privateLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                        {10, 20, 60, 220});
        m_lobbypasswordInput->SetVisible(true);
        isPrivate = true;
      });
      m_privateLobbyButton->SetColors({100, 100, 100, 255},
                                      {150, 150, 150, 255}, {80, 80, 80, 255});

      m_saveButton = GetUI()->AddElement<UIButton>(350, 520, 120, 40, "SAVE");
      m_saveButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        std::string lobbyName = m_lobbyNameInput->GetText();
        std::string passwordInput = m_lobbypasswordInput->GetText();
        if (lobbyName.empty()) {
          m_lobbyNameInput->Focus();
          return;
        }
        std::string playerName =
            GetSceneData().Get<std::string>("playerName", "");
        Action lobbyCreate{ActionType::LOBBY_CREATE,
                           LobbyCreate{lobbyName, playerName, passwordInput,
                                       maxPlayers, difficulty}};
        GetNetwork()->SendAction(lobbyCreate);
      });
      m_saveButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                              {80, 80, 80, 255});

      m_lobbyNameInput =
          GetUI()->AddElement<UITextInput>(250, 150, 300, 50, "Lobby Name");
      m_lobbypasswordInput =
          GetUI()->AddElement<UITextInput>(250, 210, 300, 50, "Lobby Password");
      m_lobbypasswordInput->SetVisible(false);

      m_isInitialized = true;
    } catch (const std::exception& e) {
      m_isInitialized = false;
    }
  }

  void OnExit() override {
    m_entities.clear();
    GetUI()->Clear();
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;
    Event e = GetNetwork()->PopEvent();
    if (e.type == EventType::LOBBY_JOIN_RESPONSE) {
      const auto* data = std::get_if<LOBBY_JOIN_RESPONSE>(&e.data);
      if (data->success == 0) return;
      GetSceneData().Set("playerId", data->playerId);
      ChangeScene("lobbyInfoPlayer");
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    // RenderSpritesLayered();
    // GetUI()->Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (GetUI()->HandleEvent(event)) {
      return;
    }
  }

  std::unordered_map<uint16_t, Entity>& GetPlayers() override {
    std::unordered_map<uint16_t, Entity> list;
    return list;
  }
};

extern "C" {
    Scene* CreateScene();
}
