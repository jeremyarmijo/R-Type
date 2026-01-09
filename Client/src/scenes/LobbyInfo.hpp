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
  UIButton* m_backButton;

  uint8_t difficulty = 1;
  bool isReady = false;
  std::string password = "";

  std::vector<PlayerInfo> m_playersInfo;
  std::string m_lobbyName;
  uint8_t m_playerMax;
  uint8_t m_difficulty;

  void RefreshPlayerListUI() {
    GetUI().Clear();

    auto* title = GetUI().AddElement<UIText>(
        50, 40, "LOBBY: " + m_lobbyName, "", 50, SDL_Color{255, 255, 255, 255});

    std::string countStr =
        "MAX PLAYER: " + std::to_string(m_playersInfo.size()) + "/" +
        std::to_string(static_cast<int>(m_playerMax));
    auto* pMax = GetUI().AddElement<UIText>(600, 40, countStr, "", 20,
                                            SDL_Color{255, 255, 255, 255});

    std::string diffStr =
        "DIFFICULTY: " + std::to_string(static_cast<int>(m_difficulty));
    auto* diff = GetUI().AddElement<UIText>(600, 80, diffStr, "", 20,
                                            SDL_Color{255, 255, 255, 255});

    title->SetVisible(true);
    title->SetLayer(10);
    pMax->SetVisible(true);
    pMax->SetLayer(11);
    diff->SetVisible(true);
    diff->SetLayer(11);

    m_readyButton = GetUI().AddElement<UIButton>(550, 500, 250, 80, "READY");
    m_readyButton->SetLayer(9);
    m_readyButton->SetOnClick([this]() {
      std::cout << "Ready button clicked!" << std::endl;
      GetAudio().PlaySound("button");
      Action playerR{ActionType::PLAYER_READY, PlayerReady{isReady}};
      if (isReady == false)
        isReady = true;
      else
        isReady = false;
      GetNetwork().SendAction(playerR);
    });
    m_readyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                             {10, 20, 60, 220});

    int yPos = 150;
    std::cout << "\n--- Current Players in Lobby ---" << std::endl;

    for (const auto& player : m_playersInfo) {
      std::cout << "Player: " << player.username << " (ID: " << player.playerId
                << ") " << (player.ready ? "[READY]" : "[NOT READY]")
                << std::endl;

      std::string label =
          player.username + (player.ready ? " - READY" : " - NOT READY");
      SDL_Color textColor = player.ready ? SDL_Color{0, 255, 0, 255}
                                         : SDL_Color{255, 255, 255, 255};

      auto* pText =
          GetUI().AddElement<UIText>(100, yPos, label, "", 24, textColor);
      pText->SetVisible(true);
      pText->SetLayer(10);
      yPos += 40;
    }
    std::cout << "--------------------------------\n" << std::endl;
  }

 public:
  LobbyInfoPlayer(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "lobbyInfoPlayer"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_readyButton(nullptr),
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
      auto* text =
          GetUI().AddElement<UIText>(50, 40, "LOBBY: " + m_lobbyName, "", 50,
                                     SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_readyButton = GetUI().AddElement<UIButton>(550, 500, 250, 80, "READY");
      m_readyButton->SetLayer(9);
      m_readyButton->SetOnClick([this]() {
        std::cout << "Ready button clicked!" << std::endl;
        GetAudio().PlaySound("button");
        if (isReady == false)
          isReady = true;
        else
          isReady = false;

        Action playerR{ActionType::PLAYER_READY, PlayerReady{isReady}};
        GetNetwork().SendAction(playerR);
      });
      m_readyButton->SetColors({20, 40, 100, 200}, {30, 60, 150, 255},
                               {10, 20, 60, 220});

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
    if (e.type == EventType::LOBBY_UPDATE) {
      const auto* data = std::get_if<LOBBY_UPDATE>(&e.data);
      if (data) {
        this->m_playersInfo = data->playerInfo;
        this->m_lobbyName = data->name;
        this->m_playerMax = data->maxPlayers;
        this->m_difficulty = data->difficulty;
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
