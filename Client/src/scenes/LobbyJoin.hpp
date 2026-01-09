#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"

class LobbyJoin : public Scene {
 private:
  bool m_isInitialized;
  std::vector<Entity> m_entities;
  std::vector<Lobbies> m_lobbies;

  UIButton* m_publicLobbyButton;
  UIButton* m_privateLobbyButton;
  UIButton* m_backButton;

  uint16_t m_PlayerId;
  bool isPrivate;
  float m_refreshTimer = 0.0f;
  const float REFRESH_INTERVAL = 5.0f;
  bool m_needsUIRefresh;

  void SendLobbyListRequest() {
    Action listReq{ActionType::LOBBY_LIST_REQUEST,
                   LobbyListRequest{m_PlayerId}};
    GetNetwork().SendAction(listReq);
  }

  void RefreshLobbyListUI() {
    GetUI().Clear();

    auto* title = GetUI().AddElement<UIText>(50, 40, "AVAILABLE LOBBIES", "",
                                             50, SDL_Color{255, 255, 255, 255});
    title->SetVisible(true);
    title->SetLayer(10);

    m_backButton = GetUI().AddElement<UIButton>(50, 100, 100, 40, "BACK");
    m_backButton->SetLayer(9);
    m_backButton->SetOnClick([this]() {
      GetAudio().PlaySound("button");
      ChangeScene("lobby");
    });
    m_backButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                            {130, 30, 30, 255});

    int yOffset = 150;
    const int buttonWidth = 600;
    const int buttonHeight = 60;

    m_publicLobbyButton =
        GetUI().AddElement<UIButton>(550, 100, 120, 40, "PUBLIC");
    m_publicLobbyButton->SetLayer(9);
    m_publicLobbyButton->SetOnClick([this]() {
      std::cout << "Public filter clicked!" << std::endl;
      GetAudio().PlaySound("button");

      isPrivate = false;
      SendLobbyListRequest();
    });
    m_publicLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                   {10, 20, 60, 220});
    if (isPrivate) {
      m_publicLobbyButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                     {80, 80, 80, 255});
    }

    m_privateLobbyButton =
        GetUI().AddElement<UIButton>(660, 100, 120, 40, "PRIVATE");
    m_privateLobbyButton->SetLayer(9);
    m_privateLobbyButton->SetOnClick([this]() {
      GetAudio().PlaySound("button");

      isPrivate = true;
      SendLobbyListRequest();
    });
    m_privateLobbyButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                    {80, 80, 80, 255});
    if (isPrivate) {
      m_privateLobbyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                                      {10, 20, 60, 220});
    }

    for (const auto& lobby : m_lobbies) {
      if (isPrivate && !lobby.hasPassword) continue;
      if (!isPrivate && lobby.hasPassword) continue;
      std::string status = lobby.isStarted ? "[IN GAME]" : "[WAITING]";
      std::string playerCount =
          std::to_string(static_cast<int>(lobby.playerCount)) + "/" +
          std::to_string(static_cast<int>(lobby.maxPlayers));
      std::string buttonLabel =
          lobby.name + "   " + status + "   " + playerCount;

      auto* btn = GetUI().AddElement<UIButton>(100, yOffset, buttonWidth,
                                               buttonHeight, buttonLabel);
      btn->SetLayer(10);

      if (lobby.isStarted || lobby.playerCount >= lobby.maxPlayers) {
        btn->SetColors({100, 100, 100, 255}, {120, 120, 120, 255},
                       {80, 80, 80, 255});
      }

      btn->SetOnClick([this, lobby]() {
        GetAudio().PlaySound("button");
        std::string pName = GetSceneData().Get<std::string>("playerName", "");
        if (isPrivate) {
          GetSceneData().Set("lobbyId", lobby.lobbyId);
          GetSceneData().Set("lobbyName", lobby.name);
          ChangeScene("lobbyPassword");
          return;
        }
        Action joinAction{ActionType::LOBBY_JOIN_REQUEST,
                          LobbyJoinRequest{lobby.lobbyId, pName, ""}};
        GetNetwork().SendAction(joinAction);
      });

      yOffset += 70;
    }

    m_needsUIRefresh = false;
  }

 public:
  LobbyJoin(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "lobbyjoin"),
        m_isInitialized(false),
        m_publicLobbyButton(nullptr),
        m_privateLobbyButton(nullptr),
        m_backButton(nullptr),
        m_refreshTimer(0.0f),
        m_needsUIRefresh(false) {}

  void OnEnter() override {
    try {
      m_isInitialized = false;
      m_refreshTimer = 0.0f;
      m_needsUIRefresh = false;

      TextureManager& textures = GetTextures();
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }

      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);

      m_PlayerId = GetSceneData().Get<uint16_t>("playerId", 0);

      SendLobbyListRequest();

      m_isInitialized = true;
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  void OnExit() override {
    m_entities.clear();
    GetUI().Clear();
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    m_refreshTimer += deltaTime;
    if (m_refreshTimer >= REFRESH_INTERVAL) {
      SendLobbyListRequest();
      m_refreshTimer = 0.0f;
    }

    Event e = GetNetwork().PopEvent();

    if (e.type == EventType::LOBBY_LIST_RESPONSE) {
      const auto* data = std::get_if<LOBBY_LIST_RESPONSE>(&e.data);
      if (data) {
        m_lobbies = data->lobbies;
        m_needsUIRefresh = true;
      }
    }

    if (e.type == EventType::LOBBY_JOIN_RESPONSE) {
      const auto* data = std::get_if<LOBBY_JOIN_RESPONSE>(&e.data);
      if (data && data->success) {
        ChangeScene("lobbyInfoPlayer");
      }
    }
    if (m_needsUIRefresh) {
      RefreshLobbyListUI();
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    RenderSpritesLayered();
    GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override { GetUI().HandleEvent(event); }
};
