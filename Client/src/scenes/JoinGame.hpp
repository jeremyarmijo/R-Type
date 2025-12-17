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

class JoinGame : public Scene {
 private:
  bool m_isInitialized;
  bool m_settingsTransition;
  std::vector<Entity> m_entities;

  UITextInput* m_usernameInput;
  UITextInput* m_serverInput;
  UIButton* m_joinButton;
  UIButton* m_backButton;

 public:
  JoinGame(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "menu"),
        m_isInitialized(false),
        m_settingsTransition(false),
        m_usernameInput(nullptr),
        m_serverInput(nullptr),
        m_joinButton(nullptr),
        m_backButton(nullptr) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING MAIN MENU SCENE ===" << std::endl;

    try {
      m_isInitialized = false;

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

      std::cout << "Creating UI Elements..." << std::endl;
      auto* text = GetUI().AddElement<UIText>(50, 40, "R-Type", "", 50,
                                              SDL_Color{255, 255, 255, 255});
      text->SetVisible(true);
      text->SetLayer(10);

      m_usernameInput = GetUI().AddElement<UITextInput>(
          70, 125, 300, 50, "Username (e.g., Space Cowboy)");
      m_usernameInput->SetMaxLength(50);
      m_usernameInput->SetText("Space Cowboy");
      m_usernameInput->SetTextColor({255, 255, 255, 255});
      m_usernameInput->SetBackgroundColor({40, 40, 50, 255});
      m_usernameInput->SetBorderColor({100, 100, 120, 255},
                                      {100, 150, 255, 255});
      m_usernameInput->SetLayer(9);
      m_usernameInput->SetVisible(true);

      m_serverInput = GetUI().AddElement<UITextInput>(
          70, 200, 300, 50, "Server IP (e.g., 127.0.0.1)");
      m_serverInput->SetMaxLength(50);
      m_serverInput->SetText("127.0.0.1");
      m_serverInput->SetTextColor({255, 255, 255, 255});
      m_serverInput->SetBackgroundColor({40, 40, 50, 255});
      m_serverInput->SetBorderColor({100, 100, 120, 255}, {100, 150, 255, 255});
      m_serverInput->SetLayer(9);
      m_serverInput->SetVisible(true);

      m_joinButton =
          GetUI().AddElement<UIButton>(70, 275, 200, 50, "Join Game");
      m_joinButton->SetLayer(9);
      m_joinButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        std::string serverIP = m_serverInput->GetText();
        std::string username = m_usernameInput->GetText();

        if (serverIP.empty()) {
          std::cout << "ERROR: Please enter a server IP!" << std::endl;
          m_serverInput->Focus();
          return;
        }
        if (username.empty()) {
          std::cout << "ERROR: Please enter a username!" << std::endl;
          m_usernameInput->Focus();
          return;
        }

        GetNetwork().Connect(serverIP, 4242);
        Action loginRequest{ActionType::LOGIN_REQUEST,
                            LoginReq{username, "hashedpassword"}};
        GetNetwork().SendAction(loginRequest);
      });
      m_joinButton->SetColors({50, 150, 50, 255}, {70, 170, 70, 255},
                              {30, 130, 30, 255});
      m_joinButton->SetVisible(true);

      m_backButton = GetUI().AddElement<UIButton>(70, 350, 200, 50, "Back");
      m_backButton->SetLayer(9);
      m_backButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        std::cout << "Back to main menu..." << std::endl;
        ChangeScene("menu");
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
    std::cout << "\n=== EXITING JOIN GAME SCENE ===" << std::endl;

    m_entities.clear();
    m_isInitialized = false;

    std::cout << "Join game cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    Event e = GetNetwork().PopEvent();
    if (e.type == EventType::LOGIN_RESPONSE) {
      const auto* data = std::get_if<LOGIN_RESPONSE>(&e.data);
      if (data->success == 0) return;
      GetSceneData().Set("playerId", data->playerId);
      ChangeScene("wait");
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
