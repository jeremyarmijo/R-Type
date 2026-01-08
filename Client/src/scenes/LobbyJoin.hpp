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
  uint16_t m_PlayerId;
  float m_refreshTimer = 0.0f;
  const float REFRESH_INTERVAL = 5.0f;

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

    int yOffset = 150;
    const int buttonWidth = 600;
    const int buttonHeight = 60;

    for (const auto& lobby : m_lobbies) {
      std::string status = lobby.isStarted ? "[IN GAME]" : "[WAITING]";
      std::string playerCount = std::to_string((int)lobby.playerCount) + "/" +
                                std::to_string((int)lobby.maxPlayers);
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
        std::cout << "LOBBBBYY ID:" << int(lobby.lobbyId) << std::endl;
        Action joinAction{ActionType::LOBBY_JOIN_REQUEST,
                          LobbyJoinRequest{lobby.lobbyId, lobby.name, ""}};
        GetNetwork().SendAction(joinAction);
      });

      yOffset += 70;
    }
  }

 public:
  LobbyJoin(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "lobbyjoin"),
        m_isInitialized(false),
        m_refreshTimer(0.0f) {}

  void OnEnter() override {
    try {
      m_isInitialized = false;
      m_refreshTimer = 0.0f;

      TextureManager& textures = GetTextures();
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }
      m_entities.push_back(
          m_engine->CreateSprite("background", {400, 300}, -10));

      m_PlayerId = GetSceneData().Get<uint16_t>("playerId", 0);

      SendLobbyListRequest();
      RefreshLobbyListUI();

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
    /*if (m_refreshTimer >= REFRESH_INTERVAL) {
        SendLobbyListRequest();
        m_refreshTimer = 0.0f;
    }*/

    Event e = GetNetwork().PopEvent();

    if (e.type == EventType::LOBBY_LIST_RESPONSE) {
      const auto* data = std::get_if<LOBBY_LIST_RESPONSE>(&e.data);
      if (data) {
        m_lobbies = data->lobbies;
        RefreshLobbyListUI();
      }
    }

    if (e.type == EventType::LOBBY_JOIN_RESPONSE) {
      const auto* data = std::get_if<LOBBY_JOIN_RESPONSE>(&e.data);
      if (data && data->success) {
        ChangeScene("lobbyInfoPlayer");
      }
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    RenderSpritesLayered();
    GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    GetUI().HandleEvent(event);
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
      ChangeScene("menu");
    }
  }
};
