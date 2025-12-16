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
        PlayTransition();
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

      m_usernameInput = GetUI().AddElement<UITextInput>(
          70, 125, 300, 50, "Username (e.g., Space Cowboy)");
      m_usernameInput->SetMaxLength(50);
      m_usernameInput->SetText("Space Cowboy");
      m_usernameInput->SetTextColor({255, 255, 255, 255});
      m_usernameInput->SetBackgroundColor({40, 40, 50, 255});
      m_usernameInput->SetBorderColor({100, 100, 120, 255},
                                      {100, 150, 255, 255});
      m_usernameInput->SetLayer(9);
      m_usernameInput->SetVisible(false);

      m_serverInput = GetUI().AddElement<UITextInput>(
          70, 200, 300, 50, "Server IP (e.g., 127.0.0.1)");
      m_serverInput->SetMaxLength(50);
      m_serverInput->SetText("127.0.0.1");
      m_serverInput->SetTextColor({255, 255, 255, 255});
      m_serverInput->SetBackgroundColor({40, 40, 50, 255});
      m_serverInput->SetBorderColor({100, 100, 120, 255}, {100, 150, 255, 255});
      m_serverInput->SetLayer(9);
      m_serverInput->SetVisible(false);

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
      m_joinButton->SetVisible(false);

      m_backButton = GetUI().AddElement<UIButton>(70, 350, 200, 50, "Back");
      m_backButton->SetLayer(9);
      m_backButton->SetOnClick([this]() {
        GetAudio().PlaySound("button");
        std::cout << "Back to main menu..." << std::endl;
        BackToMainMenu();
      });
      m_backButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                              {130, 30, 30, 255});
      m_backButton->SetVisible(false);

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
    m_isInitialized = false;

    std::cout << "Menu cleanup complete" << std::endl;
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

  void PlayTransition() {
    std::cout << "=== TRANSITIONING TO JOIN GAME SCREEN ===" << std::endl;

    m_playButton->SetVisible(false);
    m_settingsButton->SetVisible(false);
    m_quitButton->SetVisible(false);

    m_usernameInput->SetVisible(true);
    m_serverInput->SetVisible(true);
    m_joinButton->SetVisible(true);
    m_backButton->SetVisible(true);

    m_serverInput->Focus();

    std::cout << "Join game screen active" << std::endl;
  }

  void BackToMainMenu() {
    std::cout << "=== RETURNING TO MAIN MENU ===" << std::endl;

    if (m_serverInput->IsFocused()) {
      m_serverInput->Unfocus();
    }

    m_playButton->SetVisible(true);
    m_settingsButton->SetVisible(true);
    m_quitButton->SetVisible(true);

    m_usernameInput->SetVisible(false);
    m_serverInput->SetVisible(false);
    m_joinButton->SetVisible(false);
    m_backButton->SetVisible(false);

    std::cout << "Main menu active" << std::endl;
  }
};
