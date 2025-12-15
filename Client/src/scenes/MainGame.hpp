#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include "inputs/InputSystem.hpp"
#include "scene/SceneManager.hpp"
#include "settings/MultiplayerSkinManager.hpp"
#include "ui/UIManager.hpp"
#include "ui/UISolidColor.hpp"
#include "ui/UIText.hpp"
#include "systems/WeaponSystem.hpp"
#include "systems/ProjectileSystem.hpp"

class MyGameScene : public Scene {
 private:
  std::vector<Entity> m_entities;
  Entity m_localPlayer;
  std::vector<Entity> m_backGrounds;
  std::vector<Entity> m_otherPlayers;
  int m_score;
  bool m_isInitialized;
  MultiplayerSkinManager m_skinManager;

 public:
  MyGameScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "game"),
        m_score(0),
        m_isInitialized(false) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING GAME SCENE ===" << std::endl;

    try {
      // Clear old data
      m_entities.clear();
      m_otherPlayers.clear();
      m_score = 0;
      m_isInitialized = false;

      TextureManager& textures = GetTextures();
      AnimationManager& animations = GetAnimations();

      std::cout << "Loading textures..." << std::endl;
      if (!textures.GetTexture("player")) {
        textures.LoadTexture("player", "Client/assets/player.png");
      }
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "Client/assets/bg.jpg");
      }

      std::cout << "Creating background..." << std::endl;
      Entity bg1 = m_engine->CreateSprite("background", {640, 360}, -10);
      Entity bg2 = m_engine->CreateSprite("background", {1920, 360}, -10);
      m_backGrounds.push_back(bg1);
      m_backGrounds.push_back(bg2);

      PlayerSettings& settings = m_engine->GetPlayerSettings();
      std::string selectedSkinAnim = settings.GetSelectedSkinAnimation();
      m_skinManager.SetLocalPlayerSkin(settings.GetSelectedSkin());

      std::cout << "Creating player with skin: " << selectedSkinAnim
                << std::endl;
      m_localPlayer = m_engine->CreatePlayer("player", selectedSkinAnim,
                                             {200, 300}, 250.0f);
      auto& transform =
          GetRegistry().get_components<Transform>()[m_localPlayer];
      if (transform) {
        transform->scale = {2.0f, 2.0f};
      }
      m_entities.push_back(m_localPlayer);

      if (!GetRegistry().is_entity_valid(m_localPlayer)) {
        std::cerr << "ERROR: Player entity is invalid after creation!"
                  << std::endl;
        return;
      }

      // UI Elements
      auto* scoreText = GetUI().AddElement<UIText>(10, 10, "Score: 0", "", 20);
      scoreText->SetLayer(100);

      auto* healthBar = GetUI().AddElement<UISolidColor>(
          10, 40, 200, 20, (SDL_Color){200, 50, 50, 255});
      healthBar->SetLayer(100);
      auto* healthBarBG = GetUI().AddElement<UISolidColor>(
          8, 38, 204, 24, (SDL_Color){200, 200, 200, 255});
      healthBar->SetLayer(99);

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }

    std::cout << "=================================\n" << std::endl;
  }

  void OnExit() override {
    std::cout << "\n=== EXITING GAME SCENE ===" << std::endl;
    m_entities.clear();
    m_otherPlayers.clear();
    m_skinManager.Clear();
    m_isInitialized = false;
    std::cout << "Game scene cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    auto& weapons = GetRegistry().get_components<Weapon>();
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& projectiles = GetRegistry().get_components<Projectile>();
    auto& colliders = GetRegistry().get_components<BoxCollider>();

    // Weapon systems
    weapon_cooldown_system(GetRegistry(), weapons, deltaTime);
    weapon_reload_system(GetRegistry(), weapons, deltaTime);

    // Weapon firing system - vérifie si le joueur appuie sur SPACE
    weapon_firing_system(GetRegistry(), weapons, transforms,
    [this](size_t entityId) -> bool {
      auto& playerEntity = GetRegistry().get_components<PlayerEntity>();
      if (entityId < playerEntity.size() &&
          playerEntity[entityId].has_value()) {
        return GetInput().IsKeyPressed(SDL_SCANCODE_SPACE);
      }
      return false;
    }, deltaTime);

    // Projectile systems
    projectile_lifetime_system(GetRegistry(), projectiles, deltaTime);
    projectile_collision_system(GetRegistry(), transforms, colliders,
      projectiles);

    MoveBackground(deltaTime);
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
        std::cout << "Quitting to Main Menu..." << std::endl;
        ChangeScene("menu");
      }
    }
  }

  void SpawnOtherPlayer(int playerIndex, Vector2 position) {
    PlayerSkin assignedSkin = m_skinManager.AssignSkinForPlayer(playerIndex);
    std::string skinAnimation = PlayerSettings::GetSkinAnimation(assignedSkin);

    std::cout << "Spawning player " << playerIndex << " at (" << position.x
              << ", " << position.y << ")"
              << " with skin: " << PlayerSettings::GetSkinName(assignedSkin)
              << std::endl;

    Entity otherPlayer =
        m_engine->CreateAnimatedSprite("player", position, skinAnimation);
    if (playerIndex >= static_cast<int>(m_otherPlayers.size())) {
      m_otherPlayers.resize(playerIndex + 1);
    }
    m_otherPlayers[playerIndex] = otherPlayer;
    m_entities.push_back(otherPlayer);

    std::cout << "Other player " << playerIndex << " spawned successfully"
              << std::endl;
  }

  void RemoveOtherPlayer(int playerIndex) {
    if (playerIndex < static_cast<int>(m_otherPlayers.size())) {
      Entity playerEntity = m_otherPlayers[playerIndex];

      GetRegistry().kill_entity(playerEntity);
      m_skinManager.RemovePlayer(playerIndex);
      m_entities.erase(
          std::remove(m_entities.begin(), m_entities.end(), playerEntity),
          m_entities.end());

      std::cout << "Removed player " << playerIndex << std::endl;
    }
  }

  void UpdateOtherPlayerPosition(int playerIndex, Vector2 position) {
    if (playerIndex < static_cast<int>(m_otherPlayers.size())) {
      Entity playerEntity = m_otherPlayers[playerIndex];
      auto& transforms = GetRegistry().get_components<Transform>();

      if (playerEntity < transforms.size() &&
          transforms[playerEntity].has_value()) {
        transforms[playerEntity]->position = position;
      }
    }
  }

  void MoveBackground(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();

    for (auto& bg : m_backGrounds) {
      if (bg < transforms.size() && transforms[bg].has_value()) {
        transforms[bg]->position.x -= 80.f * deltaTime;
        if (transforms[bg]->position.x <= -640)
          transforms[bg]->position.x = 1920;
      }
    }
  }
};
