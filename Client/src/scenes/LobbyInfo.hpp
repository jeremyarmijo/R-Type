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

class LobbyInfoPlayer : public Scene {
 private:
  bool m_isInitialized;
  bool m_settingsTransition;
  std::vector<Entity> m_entities;

  UIButton* m_readyButton;
  UIButton* m_leaveButton;
  UIButton* m_chatContainer;
  UIButton* m_littelChatContainer;
  UIButton* m_hiddeChat;
  UIButton* m_showChat;
  UITextInput* m_chatInput;
  UIButton* m_sendChat;

  std::vector<std::string> message;

  bool isReady = false;
  bool isLittelChat = true;
  bool m_asStarted = false;
  std::vector<PlayerInfo> m_playersInfo;
  std::string m_lobbyName;
  uint8_t m_playerMax;
  uint8_t m_difficulty;
  uint8_t m_lobbyId;

  void UpdateChatDisplay(int maxMessages) {
    int startY = 540;
    int xPos = 15;

    int count = 0;

    if (message.size() > 14) {
      message.erase(message.begin());
    }

    if (isLittelChat) maxMessages = 1;

    RefreshPlayerListUI();
    for (auto it = message.rbegin();
         it != message.rend() && count < maxMessages; ++it) {
      auto* msgText = GetUI().AddElement<UIText>(xPos, startY, *it, "", 18,
                                                 SDL_Color{255, 255, 255, 255});
      msgText->SetLayer(10000);

      startY -= 25;
      count++;
    }
  }

  void RefreshPlayerListUI() {
    GetUI().Clear();

    auto* title = GetUI().AddElement<UIText>(
        50, 40, "LOBBY: " + m_lobbyName, "", 45, SDL_Color{255, 255, 255, 255});
    title->SetLayer(10);

    std::string countStr = "PLAYERS: " + std::to_string(m_playersInfo.size()) +
                           " / " +
                           std::to_string(static_cast<int>(m_playerMax));
    auto* pMax = GetUI().AddElement<UIText>(550, 45, countStr, "", 22,
                                            SDL_Color{200, 200, 200, 255});

    std::string diffStr =
        "DIFFICULTY: " + std::to_string(static_cast<int>(m_difficulty));
    auto* diff = GetUI().AddElement<UIText>(550, 75, diffStr, "", 22,
                                            SDL_Color{200, 200, 200, 255});
    if (!m_asStarted) {
      m_readyButton = GetUI().AddElement<UIButton>(
          520, 500, 220, 60, isReady ? "NOT READY" : "READY");
      m_readyButton->SetLayer(9);
      m_readyButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        isReady = !isReady;
        GetSceneData().Set<bool>("isSpectator", false);
        Action playerR{ActionType::PLAYER_READY, PlayerReady{isReady}};
        GetNetwork().SendAction(playerR);
        RefreshPlayerListUI();
      });
    } else {
      m_readyButton =
          GetUI().AddElement<UIButton>(520, 500, 220, 60, "SPECTATE");
      m_readyButton->SetLayer(9);
      m_readyButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        Action playerR{ActionType::PLAYER_READY, PlayerReady{true}};
        GetSceneData().Set<bool>("isSpectator", true);
        GetNetwork().SendAction(playerR);
      });
    }

    if (isReady) {
      m_readyButton->SetColors({180, 40, 40, 255}, {200, 60, 60, 255},
                               {150, 30, 30, 220});
    } else {
      m_readyButton->SetColors({40, 150, 40, 255}, {60, 180, 60, 255},
                               {30, 120, 30, 220});
    }

    m_chatContainer = GetUI().AddElement<UIButton>(5, 200, 500, 700, "   ");
    m_chatContainer->SetLayer(100);
    m_chatContainer->SetOnClick([this]() {});
    m_chatContainer->SetColors({100, 100, 100, 255}, {100, 100, 100, 255},
                               {100, 100, 100, 255});

    m_littelChatContainer =
        GetUI().AddElement<UIButton>(5, 530, 430, 70, "   ");
    m_littelChatContainer->SetLayer(100);
    m_littelChatContainer->SetOnClick([this]() {});
    m_littelChatContainer->SetColors({100, 100, 100, 255}, {100, 100, 100, 255},
                                     {100, 100, 100, 255});

    m_hiddeChat = GetUI().AddElement<UIButton>(485, 200, 20, 20, "-");
    m_hiddeChat->SetLayer(1000);
    m_hiddeChat->SetOnClick([this]() {
      isLittelChat = true;
      UpdateChatDisplay(1);
    });
    m_hiddeChat->SetColors({45, 45, 55, 255}, {55, 55, 65, 255},
                           {35, 35, 45, 255});

    m_showChat = GetUI().AddElement<UIButton>(414, 531, 20, 20, "+");
    m_showChat->SetLayer(1000);
    m_showChat->SetOnClick([this]() {
      isLittelChat = false;
      UpdateChatDisplay(14);
    });
    m_showChat->SetColors({45, 45, 55, 255}, {55, 55, 65, 255},
                          {35, 35, 45, 255});

    m_sendChat = GetUI().AddElement<UIButton>(350, 568, 61, 27, "->");
    m_sendChat->SetLayer(100000);
    m_sendChat->SetOnClick([this]() {
      auto chatMessage = m_chatInput->GetText();
      m_chatInput->SetText("");
      if (!chatMessage.empty() && chatMessage.size() < 25) {
        std::string playerName =
            GetSceneData().Get<std::string>("playerName", "");
        Action leaveReq{ActionType::MESSAGE,
                        Message{0, playerName, chatMessage}};
        GetNetwork().SendAction(leaveReq);
      }
    });
    m_sendChat->SetColors({45, 45, 55, 255}, {55, 55, 65, 255},
                          {35, 35, 45, 255});

    m_chatInput = GetUI().AddElement<UITextInput>(7, 567, 340, 30, "Chat...");
    m_chatInput->SetMaxLength(50);
    m_chatInput->SetText("");
    m_chatInput->SetTextColor({255, 255, 255, 255});
    m_chatInput->SetBackgroundColor({40, 40, 50, 255});
    m_chatInput->SetBorderColor({100, 100, 120, 255}, {100, 150, 255, 255});
    m_chatInput->SetLayer(10000);
    m_chatInput->SetVisible(true);

    m_leaveButton = GetUI().AddElement<UIButton>(560, 450, 180, 45, "<- LEAVE");
    m_leaveButton->SetLayer(9);
    m_leaveButton->SetOnClick([this]() {
      GetAudio().PlaySound("button");
      uint16_t pId = GetSceneData().Get<uint16_t>("playerId", 0);
      Action leaveReq{ActionType::LOBBY_LEAVE, LobbyLeave{pId}};
      GetNetwork().SendAction(leaveReq);
      ChangeScene("lobby");
    });

    if (isLittelChat) {
      m_hiddeChat->SetVisible(false);
      m_chatContainer->SetVisible(false);
      m_littelChatContainer->SetVisible(true);
      m_showChat->SetVisible(true);
    } else {
      m_showChat->SetVisible(false);
      m_littelChatContainer->SetVisible(false);
      m_chatContainer->SetVisible(true);
      m_hiddeChat->SetVisible(true);
    }

    m_leaveButton->SetColors({100, 100, 100, 255}, {130, 130, 130, 255},
                             {80, 80, 80, 220});

    auto* header = GetUI().AddElement<UIText>(80, 130, "PLAYER NAME", "", 18,
                                              SDL_Color{150, 150, 150, 255});
    auto* status = GetUI().AddElement<UIText>(450, 130, "STATUS", "", 18,
                                              SDL_Color{150, 150, 150, 255});

    int yPos = 170;
    for (const auto& player : m_playersInfo) {
      SDL_Color statusColor = player.ready ? SDL_Color{0, 255, 100, 255}
                                           : SDL_Color{255, 255, 255, 255};

      auto* pName = GetUI().AddElement<UIText>(
          80, yPos, player.username, "", 26, SDL_Color{255, 255, 255, 255});

      if (m_asStarted) {
        auto* pStatus = GetUI().AddElement<UIText>(450, yPos, "IN GAME", "", 24,
                                                   statusColor);
        pStatus->SetLayer(10);
      } else {
        auto* pStatus = GetUI().AddElement<UIText>(
            450, yPos, player.ready ? "READY" : "WAITING...", "", 24,
            statusColor);
        pStatus->SetLayer(10);
      }

      pName->SetLayer(10);
      yPos += 50;
    }
  }

 public:
  LobbyInfoPlayer(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "lobbyInfoPlayer"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_readyButton(nullptr),
        m_leaveButton(nullptr) {}

  void OnEnter() override {
    try {
      m_isInitialized = false;
      TextureManager& textures = GetTextures();
      AudioManager& audio = GetAudio();

      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }

      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);

      RefreshPlayerListUI();
      m_isInitialized = true;
    } catch (const std::exception& e) {
      m_isInitialized = false;
    }
  }

  void OnExit() override {
    m_entities.clear();
    GetUI().Clear();
    isReady = false;
    m_isInitialized = false;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    Event e = GetNetwork().PopEvent();
    if (e.type == EventType::LOBBY_UPDATE) {
      const auto* data = std::get_if<LOBBY_UPDATE>(&e.data);
      if (data) {
        this->m_playersInfo = data->playerInfo;
        this->m_lobbyName = data->name;
        this->m_playerMax = data->maxPlayers;
        this->m_difficulty = data->difficulty;
        this->m_asStarted = data->asStarted;
        RefreshPlayerListUI();
      }
    }
    if (e.type == EventType::GAME_START) {
      const auto* data = std::get_if<GAME_START>(&e.data);
      GetSceneData().Set("posX", data->playerSpawnX);
      GetSceneData().Set("posY", data->playerSpawnY);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      ChangeScene("game");
    }
    if (e.type == EventType::MESSAGE) {
      const auto* data = std::get_if<MESSAGE>(&e.data);
      message.push_back(data->playerName + " : " + data->message);
      UpdateChatDisplay(14);
    }
  }

  void Render() override {
    if (!m_isInitialized) return;
    RenderSpritesLayered();
    GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override { GetUI().HandleEvent(event); }
};
