#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <string>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "network/NetworkSubsystem.hpp"
#include "audio/AudioSubsystem.hpp"
#include "Helpers/EntityHelper.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"
#include "ui/UITextInput.hpp"

class LobbyPassword : public Scene {
 private:
  bool m_isInitialized;
  std::vector<Entity> m_entities;

  UITextInput* m_passwordInput;
  UIButton* m_joinButton;
  UIButton* m_backButton;

  uint16_t m_targetLobbyId;
  std::string m_targetLobbyName;

  void SendJoinRequest() {
    std::string password = m_passwordInput->GetText();
    std::string pName = GetSceneData().Get<std::string>("playerName", "Player");

    if (password.empty()) {
      m_passwordInput->Focus();
      return;
    }

    Action joinAction{ActionType::LOBBY_JOIN_REQUEST,
                      LobbyJoinRequest{m_targetLobbyId, pName, password}};

    GetNetwork()->SendAction(joinAction);
  }

 public:
  LobbyPassword()
      : m_isInitialized(false),
        m_passwordInput(nullptr),
        m_joinButton(nullptr),
        m_backButton(nullptr) { m_name = "lobbyPassword"; }

  void OnEnter() override {
    try {
      m_isInitialized = false;

      m_targetLobbyId = GetSceneData().Get<uint16_t>("lobbyId", 0);
      m_targetLobbyName =
          GetSceneData().Get<std::string>("lobbyName", "Unknown Lobby");

      m_entities.push_back(
          CreateSprite(GetRegistry(), "background", {400, 300}, -10));

      auto* title = GetUI()->AddElement<UIText>(70, 40, "PRIVATE LOBBY", "", 50,
                                               SDL_Color{255, 255, 255, 255});
      title->SetLayer(10);

      auto* lobbyNameDisplay =
          GetUI()->AddElement<UIText>(70, 100, "Lobby: " + m_targetLobbyName, "",
                                     25, SDL_Color{100, 200, 255, 255});
      lobbyNameDisplay->SetLayer(10);

      m_passwordInput = GetUI()->AddElement<UITextInput>(70, 180, 400, 50,
                                                        "Enter Password...");
      m_passwordInput->SetMaxLength(32);
      m_passwordInput->SetTextColor({255, 255, 255, 255});
      m_passwordInput->SetBackgroundColor({40, 40, 50, 255});
      m_passwordInput->SetBorderColor({100, 100, 120, 255},
                                      {100, 150, 255, 255});
      m_passwordInput->SetLayer(9);
      m_passwordInput->Focus();

      m_joinButton =
          GetUI()->AddElement<UIButton>(70, 300, 200, 50, "Join Lobby");
      m_joinButton->SetColors({50, 150, 50, 255}, {70, 170, 70, 255},
                              {30, 130, 30, 255});
      m_joinButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        SendJoinRequest();
      });

      m_backButton = GetUI()->AddElement<UIButton>(290, 300, 200, 50, "Back");
      m_backButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                              {130, 30, 30, 255});
      m_backButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ChangeScene("lobbyjoin");
      });

      m_isInitialized = true;
    } catch (const std::exception& e) {
      std::cerr << "LobbyPassword Error: " << e.what() << std::endl;
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
      if (data) {
        if (data && data->success) {
          ChangeScene("lobbyInfoPlayer");
        } else {
          m_passwordInput->SetText("");
        }
      }
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    // RenderSpritesLayered();
    // GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (GetUI()->HandleEvent(event)) {
      return;
    }

    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_RETURN) {
        SendJoinRequest();
      } else if (event.key.keysym.sym == SDLK_ESCAPE) {
        ChangeScene("lobbyjoin");
      }
    }
  }
};

extern "C" {
    Scene* CreateScene();
}
