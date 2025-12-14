#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include "Player/PlayerEntity.hpp"
#include "inputs/InputSystem.hpp"
#include "scene/SceneManager.hpp"
#include "settings/MultiplayerSkinManager.hpp"
#include "ui/UIManager.hpp"
#include "ui/UISolidColor.hpp"
#include "ui/UIText.hpp"

class MyGameScene : public Scene {
 private:
  std::vector<Entity> m_entities;
  Entity m_localPlayer;
  uint16_t m_localPlayerId;
  std::vector<Entity> m_backGrounds;
  std::unordered_map<uint16_t, Entity> m_otherPlayers;
  std::unordered_map<uint16_t, Entity> m_enemies;
  std::unordered_map<uint16_t, Entity> m_projectiles;
  int m_score;
  bool m_isInitialized;
  bool m_firstState;
  MultiplayerSkinManager m_skinManager;

 public:
  MyGameScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "game"),
        m_localPlayerId(0),
        m_score(0),
        m_isInitialized(false),
        m_firstState(false) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING GAME SCENE ===" << std::endl;

    try {
      m_entities.clear();
      m_otherPlayers.clear();
      m_enemies.clear();
      m_projectiles.clear();
      m_score = 0;
      m_isInitialized = false;
      m_firstState = false;

      TextureManager& textures = GetTextures();
      AnimationManager& animations = GetAnimations();

      std::cout << "Loading textures..." << std::endl;
      LoadGameTextures(textures);
      CreateGameAnimations(animations);

      std::cout << "Creating background..." << std::endl;
      Entity bg1 = m_engine->CreateSprite("background", {640, 360}, -10);
      Entity bg2 = m_engine->CreateSprite("background", {1920, 360}, -10);
      m_backGrounds.push_back(bg1);
      m_backGrounds.push_back(bg2);

      PlayerSettings& settings = m_engine->GetPlayerSettings();
      m_localPlayerId = GetSceneData().Get<uint16_t>("playerId", 0);

      std::string selectedSkinAnim = settings.GetSelectedSkinAnimation();
      m_skinManager.SetLocalPlayerSkin(settings.GetSelectedSkin(),
                                       m_localPlayerId);

      std::cout << "Creating local player (ID: " << m_localPlayerId
                << ") with skin: " << selectedSkinAnim << std::endl;

      float posX = GetSceneData().Get<float>("posX", 200.0f);
      float posY = GetSceneData().Get<float>("posY", 300.0f);

      m_localPlayer = m_engine->CreatePlayer("player", selectedSkinAnim,
                                             {posX, posY}, 250.0f);

      auto& transform =
          GetRegistry().get_components<Transform>()[m_localPlayer];
      if (transform) {
        transform->scale = {2.0f, 2.0f};
      }

      auto& playerComponent =
          GetRegistry().get_components<PlayerControlled>()[m_localPlayer];
      if (playerComponent) {
        playerComponent->player_id = m_localPlayerId;
        playerComponent->current = 100;
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
    m_enemies.clear();
    m_projectiles.clear();
    m_skinManager.Clear();
    m_isInitialized = false;
    m_firstState = false;
    std::cout << "Game scene cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;

    MoveBackground(deltaTime);
    GetEvents(deltaTime);
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

    // if (event.type == SDL_KEYDOWN) {
    //   if (event.key.keysym.sym == SDLK_r) {
    //     std::cout << "Restarting level..." << std::endl;
    //     ChangeScene("game");
    //   } else if (event.key.keysym.sym == SDLK_x) {
    //     std::cout << "Quitting to Main Menu..." << std::endl;
    //     ChangeScene("menu");
    //   }
    // }
  }

  void LoadGameTextures(TextureManager& textures) {
    if (!textures.GetTexture("player")) {
      textures.LoadTexture("player", "../Client/assets/player.png");
    }

    if (!textures.GetTexture("background")) {
      textures.LoadTexture("background", "../Client/assets/bg.jpg");
    }

    if (!textures.GetTexture("enemy1")) {
      textures.LoadTexture("enemy1", "../Client/assets/enemy1.png");
    }
    if (!textures.GetTexture("enemy2")) {
      textures.LoadTexture("enemy2", "../Client/assets/enemy2.png");
    }
    if (!textures.GetTexture("enemy3")) {
      textures.LoadTexture("enemy3", "../Client/assets/enemy3.png");
    }

    if (!textures.GetTexture("projectile_player")) {
      textures.LoadTexture("projectile_player",
                           "../Client/assets/projectile_player.png");
    }
    if (!textures.GetTexture("projectile_enemy")) {
      textures.LoadTexture("projectile_enemy",
                           "../Client/assets/projectile_enemy.png");
    }
  }

  void CreateGameAnimations(AnimationManager& animations) {
    animations.CreateAnimation("enemy1_anim", "enemy1",
                               {{{0, 0, 32, 32}, 0.1f},
                                {{32, 0, 32, 32}, 0.1f},
                                {{64, 0, 32, 32}, 0.1f}},
                               true);

    animations.CreateAnimation(
        "enemy2_anim", "enemy2",
        {{{0, 0, 48, 48}, 0.15f}, {{48, 0, 48, 48}, 0.15f}}, true);

    animations.CreateAnimation("enemy3_anim", "enemy3",
                               {{{0, 0, 64, 64}, 0.2f},
                                {{64, 0, 64, 64}, 0.2f},
                                {{128, 0, 64, 64}, 0.2f}},
                               true);

    animations.CreateAnimation(
        "projectile_player_anim", "projectile_player",
        {{{0, 0, 16, 16}, 0.05f}, {{16, 0, 16, 16}, 0.05f}}, true);

    animations.CreateAnimation(
        "projectile_enemy_anim", "projectile_enemy",
        {{{0, 0, 12, 12}, 0.1f}, {{12, 0, 12, 12}, 0.1f}}, true);
  }

  std::string GetEnemyTexture(uint8_t enemyType) const {
    switch (enemyType) {
      case 0:
        return "enemy1";
      case 1:
        return "enemy2";
      case 2:
        return "enemy3";
      default:
        return "enemy1";
    }
  }

  std::string GetEnemyAnimation(uint8_t enemyType) const {
    switch (enemyType) {
      case 0:
        return "enemy1_anim";
      case 1:
        return "enemy2_anim";
      case 2:
        return "enemy3_anim";
      default:
        return "enemy1_anim";
    }
  }

  std::string GetProjectileTexture(uint8_t projectileType) const {
    switch (projectileType) {
      case 0:
        return "projectile_player";
      case 1:
        return "projectile_enemy";
      default:
        return "projectile_player";
    }
  }

  std::string GetProjectileAnimation(uint8_t projectileType) const {
    switch (projectileType) {
      case 0:
        return "projectile_player_anim";
      case 1:
        return "projectile_enemy_anim";
      default:
        return "projectile_player_anim";
    }
  }

  void SpawnOtherPlayer(uint16_t playerId, Vector2 position) {
    if (playerId == m_localPlayerId) {
      return;
    }

    // If player already exists, don't spawn again
    if (m_otherPlayers.find(playerId) != m_otherPlayers.end()) {
      std::cout << "Player " << playerId << " already spawned, skipping"
                << std::endl;
      return;
    }

    PlayerSkin assignedSkin = m_skinManager.AssignSkinForPlayer(playerId);
    std::string skinAnimation = PlayerSettings::GetSkinAnimation(assignedSkin);

    std::cout << "Spawning player " << playerId << " at (" << position.x << ", "
              << position.y << ")"
              << " with skin: " << PlayerSettings::GetSkinName(assignedSkin)
              << std::endl;

    Entity otherPlayer =
        m_engine->CreateAnimatedSprite("player", position, skinAnimation);

    auto& transform = GetRegistry().get_components<Transform>()[otherPlayer];
    if (transform) {
      transform->scale = {2.0f, 2.0f};
    }

    m_otherPlayers[playerId] = otherPlayer;
    m_entities.push_back(otherPlayer);

    std::cout << "Other player " << playerId << " spawned successfully"
              << std::endl;
  }

  void RemoveOtherPlayer(uint16_t playerId) {
    auto it = m_otherPlayers.find(playerId);
    if (it != m_otherPlayers.end()) {
      Entity playerEntity = it->second;

      GetRegistry().kill_entity(playerEntity);
      m_skinManager.RemovePlayer(playerId);
      m_entities.erase(
          std::remove(m_entities.begin(), m_entities.end(), playerEntity),
          m_entities.end());
      m_otherPlayers.erase(it);

      std::cout << "Removed player " << playerId << std::endl;
    } else {
      std::cout << "Attempted to remove non-existent player " << playerId
                << std::endl;
    }
  }

  void UpdateOtherPlayerPosition(uint16_t playerId, Vector2 position) {
    auto it = m_otherPlayers.find(playerId);
    if (it != m_otherPlayers.end()) {
      Entity playerEntity = it->second;
      auto& transforms = GetRegistry().get_components<Transform>();

      if (playerEntity < transforms.size() &&
          transforms[playerEntity].has_value()) {
        transforms[playerEntity]->position = position;
      }
    } else {
      std::cout << "Player " << playerId << " not found, spawning..."
                << std::endl;
      SpawnOtherPlayer(playerId, position);
    }
  }

  void SpawnEnemy(uint16_t enemyId, uint8_t enemyType, Vector2 position) {
    if (m_enemies.find(enemyId) != m_enemies.end()) {
      return;
    }

    std::string texture = GetEnemyTexture(enemyType);
    std::string animation = GetEnemyAnimation(enemyType);

    std::cout << "Spawning enemy " << enemyId << " (type " << (int)enemyType
              << ") at (" << position.x << ", " << position.y << ")"
              << std::endl;

    Entity enemy = m_engine->CreateAnimatedSprite(texture, position, animation);

    auto& transform = GetRegistry().get_components<Transform>()[enemy];
    if (transform) {
      transform->scale = {2.0f, 2.0f};
    }

    m_enemies[enemyId] = enemy;
    m_entities.push_back(enemy);

    std::cout << "Enemy " << enemyId << " spawned successfully" << std::endl;
  }

  void RemoveEnemy(uint16_t enemyId) {
    auto it = m_enemies.find(enemyId);
    if (it != m_enemies.end()) {
      Entity enemyEntity = it->second;
      GetRegistry().kill_entity(enemyEntity);
      m_entities.erase(
          std::remove(m_entities.begin(), m_entities.end(), enemyEntity),
          m_entities.end());
      m_enemies.erase(it);
      std::cout << "Removed enemy " << enemyId << std::endl;
    }
  }

  void UpdateEnemyPosition(uint16_t enemyId, Vector2 position) {
    auto it = m_enemies.find(enemyId);
    if (it != m_enemies.end()) {
      Entity enemyEntity = it->second;
      auto& transforms = GetRegistry().get_components<Transform>();

      if (enemyEntity < transforms.size() &&
          transforms[enemyEntity].has_value()) {
        transforms[enemyEntity]->position = position;
      }
    }
  }

  void SpawnProjectile(uint16_t projectileId, uint8_t projectileType,
                       Vector2 position) {
    if (m_projectiles.find(projectileId) != m_projectiles.end()) {
      return;
    }

    std::string texture = GetProjectileTexture(projectileType);
    std::string animation = GetProjectileAnimation(projectileType);

    Entity projectile =
        m_engine->CreateAnimatedSprite(texture, position, animation);

    auto& transform = GetRegistry().get_components<Transform>()[projectile];
    if (transform) {
      transform->scale = {1.5f, 1.5f};
    }

    m_projectiles[projectileId] = projectile;
    m_entities.push_back(projectile);
  }

  void RemoveProjectile(uint16_t projectileId) {
    auto it = m_projectiles.find(projectileId);
    if (it != m_projectiles.end()) {
      Entity projectileEntity = it->second;
      GetRegistry().kill_entity(projectileEntity);
      m_entities.erase(
          std::remove(m_entities.begin(), m_entities.end(), projectileEntity),
          m_entities.end());
      m_projectiles.erase(it);
    }
  }

  void UpdateProjectilePosition(uint16_t projectileId, Vector2 position) {
    auto it = m_projectiles.find(projectileId);
    if (it != m_projectiles.end()) {
      Entity projectileEntity = it->second;
      auto& transforms = GetRegistry().get_components<Transform>();

      if (projectileEntity < transforms.size() &&
          transforms[projectileEntity].has_value()) {
        transforms[projectileEntity]->position = position;
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

  void UpdatePlayers(const std::vector<GAME_STATE::PlayerState>& playerStates,
                     float dt) {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& playerComponents = GetRegistry().get_components<PlayerControlled>();

    for (const auto& playerState : playerStates) {
      uint16_t playerId = playerState.playerId;

      if (playerId == m_localPlayerId) {
        if (m_localPlayer < transforms.size() &&
            transforms[m_localPlayer].has_value()) {
          transforms[m_localPlayer]->position.x = playerState.posX;
          transforms[m_localPlayer]->position.y = playerState.posY;
        }

        if (m_localPlayer < playerComponents.size() &&
            playerComponents[m_localPlayer].has_value()) {
          playerComponents[m_localPlayer]->current =
              static_cast<int>(playerState.hp);
        }
      } else {
        auto it = m_otherPlayers.find(playerId);
        if (it == m_otherPlayers.end()) {
          SpawnOtherPlayer(playerId, {playerState.posX, playerState.posY});
        } else {
          UpdateOtherPlayerPosition(playerId,
                                    {playerState.posX, playerState.posY});
        }
      }
    }

    if (m_firstState) {
      std::unordered_set<uint16_t> activePlayerIds;
      for (const auto& playerState : playerStates) {
        activePlayerIds.insert(playerState.playerId);
      }

      std::vector<uint16_t> toRemove;
      for (const auto& [playerId, entity] : m_otherPlayers) {
        if (activePlayerIds.find(playerId) == activePlayerIds.end()) {
          toRemove.push_back(playerId);
        }
      }

      for (uint16_t playerId : toRemove) {
        RemoveOtherPlayer(playerId);
      }
    }

    m_firstState = true;
  }

  void UpdateEnemies(const std::vector<GAME_STATE::EnemyState>& enemies,
                     float dt) {
    std::unordered_set<uint16_t> activeEnemyIds;

    for (const auto& enemyState : enemies) {
      uint16_t enemyId = enemyState.enemyId;
      activeEnemyIds.insert(enemyId);

      auto it = m_enemies.find(enemyId);
      if (it == m_enemies.end()) {
        SpawnEnemy(enemyId, enemyState.enemyType,
                   {enemyState.posX, enemyState.posY});
      } else {
        UpdateEnemyPosition(enemyId, {enemyState.posX, enemyState.posY});
      }
    }

    std::vector<uint16_t> toRemove;
    for (const auto& [enemyId, entity] : m_enemies) {
      if (activeEnemyIds.find(enemyId) == activeEnemyIds.end()) {
        toRemove.push_back(enemyId);
      }
    }

    for (uint16_t enemyId : toRemove) {
      RemoveEnemy(enemyId);
    }
  }

  void UpdateProjectiles(
      const std::vector<GAME_STATE::ProjectileState>& projectiles, float dt) {
    std::unordered_set<uint16_t> activeProjectileIds;

    for (const auto& projectileState : projectiles) {
      uint16_t projectileId = projectileState.projectileId;
      activeProjectileIds.insert(projectileId);

      auto it = m_projectiles.find(projectileId);
      if (it == m_projectiles.end()) {
        SpawnProjectile(projectileId, projectileState.type,
                        {projectileState.posX, projectileState.posY});
      } else {
        UpdateProjectilePosition(projectileId,
                                 {projectileState.posX, projectileState.posY});
      }
    }

    std::vector<uint16_t> toRemove;
    for (const auto& [projectileId, entity] : m_projectiles) {
      if (activeProjectileIds.find(projectileId) == activeProjectileIds.end()) {
        toRemove.push_back(projectileId);
      }
    }

    for (uint16_t projectileId : toRemove) {
      RemoveProjectile(projectileId);
    }
  }

  void GetEvents(float dt) {
    Event e = GetNetwork().PopEvent();

    std::visit(
        [&](auto&& payload) {
          using T = std::decay_t<decltype(payload)>;

          if constexpr (std::is_same_v<T, GAME_STATE>) {
            UpdatePlayers(payload.players, dt);
            UpdateEnemies(payload.enemies, dt);
            UpdateProjectiles(payload.projectiles, dt);
          } else if constexpr (std::is_same_v<T, BOSS_SPAWN>) {
          } else if constexpr (!std::is_same_v<T, BOSS_UPDATE>) {
          } else if constexpr (!std::is_same_v<T, GAME_END>) {
          } else if constexpr (!std::is_same_v<T, ENEMY_HIT>) {
          }
        },
        e.data);
  }
};
