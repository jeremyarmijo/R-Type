#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Player/PlayerEntity.hpp"
#include "inputs/InputSystem.hpp"
#include "scene/SceneManager.hpp"
#include "settings/MultiplayerSkinManager.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WeaponSystem.hpp"
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
  std::unordered_map<size_t, float> m_explosions;
  std::unordered_map<uint16_t, Entity> m_forces;
  int m_score;
  bool m_isInitialized;
  bool m_firstState;
  bool m_isAlive;
  uint32_t m_level;
  uint32_t m_wave;
  bool m_nextWave;
  UIText* m_scoreText;
  UIText* m_healthText;
  UIText* m_levelText;
  MultiplayerSkinManager m_skinManager;

 public:
  MyGameScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "game"),
        m_localPlayerId(0),
        m_score(0),
        m_isInitialized(false),
        m_firstState(false),
        m_isAlive(true),
        m_level(0),
        m_wave(0),
        m_nextWave(false),
        m_scoreText(nullptr),
        m_healthText(nullptr),
        m_levelText(nullptr) {}

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
      GetAudio().PlayMusic("game_music");

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
          GetRegistry().get_components<PlayerEntity>()[m_localPlayer];
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
      m_scoreText = GetUI().AddElement<UIText>(10, 10, "Score: 0", "", 20);
      m_scoreText->SetLayer(100);

      m_healthText = GetUI().AddElement<UIText>(10, 40, "Health: 100", "", 20);
      m_healthText->SetLayer(100);

      m_levelText = GetUI().AddElement<UIText>(10, 60, "Level: 0", "", 20);
      m_levelText->SetLayer(100);

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
    m_forces.clear();
    m_explosions.clear();
    m_skinManager.Clear();
    m_isInitialized = false;
    m_firstState = false;
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
    RemoveExplosions(deltaTime);
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
                           "../Client/assets/blueShoot.png");
    }
    if (!textures.GetTexture("projectile_enemy")) {
      textures.LoadTexture("projectile_enemy",
                           "../Client/assets/projectile_enemy.png");
    }
    if (!textures.GetTexture("explosion")) {
      textures.LoadTexture("explosion", "../Client/assets/explosion.png");
    }
    if (!textures.GetTexture("force")) {
      textures.LoadTexture("force", "../Client/assets/force.png");
    }
  }

  void CreateGameAnimations(AnimationManager& animations) {
    animations.CreateAnimation("enemy1_anim", "enemy1",
                               {{{5, 6, 20, 23}, 0.1f},
                                {{38, 6, 20, 23}, 0.1f},
                                {{71, 6, 20, 23}, 0.1f},
                                {{104, 6, 20, 23}, 0.1f},
                                {{137, 6, 20, 23}, 0.1f},
                                {{170, 6, 20, 23}, 0.1f},
                                {{203, 6, 20, 23}, 0.1f},
                                {{236, 6, 20, 23}, 0.1f}},
                               true);

    animations.CreateAnimation("boss_anim", "boss",
                               {{{27, 1711, 154, 203}, 0.6f},
                                {{189, 1711, 154, 203}, 0.5f},
                                {{351, 1711, 154, 203}, 0.6f},
                                {{189, 1711, 154, 203}, 0.5f}},
                               true);

    animations.CreateAnimation("explode_anim", "explosion",
                               {{{130, 2, 30, 30}, 0.1f},
                                {{163, 2, 30, 30}, 0.1f},
                                {{194, 2, 30, 30}, 0.1f},
                                {{228, 2, 30, 30}, 0.1f},
                                {{261, 2, 30, 30}, 0.1f},
                                {{294, 2, 30, 30}, 0.1f}},
                               true);

    animations.CreateAnimation(
        "enemy2_anim", "enemy2",
        {{{34, 34, 31, 31}, 0.15f}, {{69, 34, 31, 31}, 0.15f}}, true);

    animations.CreateAnimation("enemy3_anim", "enemy3",
                               {{{2, 67, 29, 31}, 0.2f},
                                {{35, 67, 29, 31}, 0.2f},
                                {{68, 67, 29, 31}, 0.2f},
                                {{101, 67, 29, 31}, 0.2f}},
                               true);

    animations.CreateAnimation("projectile_player_anim", "projectile_player",
                               {{{1, 0, 17, 5}, 0.1f}, {{19, 0, 17, 5}, 0.1f}},
                               true);

    animations.CreateAnimation(
        "projectile_enemy_anim", "projectile_enemy",
        {{{0, 0, 12, 12}, 0.1f}, {{12, 0, 12, 12}, 0.1f}}, true);

    animations.CreateAnimation("force_anim", "force",
                               {{{1, 1, 17, 16}, 0.08f},
                                {{19, 1, 17, 16}, 0.08f},
                                {{37, 1, 17, 16}, 0.08f},
                                {{55, 1, 17, 16}, 0.08f},
                                {{73, 1, 17, 16}, 0.08f},
                                {{91, 1, 17, 16}, 0.08f},
                                {{109, 1, 17, 16}, 0.08f},
                                {{127, 1, 17, 16}, 0.08f},
                                {{145, 1, 17, 16}, 0.08f},
                                {{163, 1, 17, 16}, 0.08f},
                                {{181, 1, 17, 16}, 0.08f},
                                {{199, 1, 17, 16}, 0.08f}},
                               true);
  }

  std::string GetEnemyTexture(uint8_t enemyType) const {
    switch (enemyType) {
      case 0:
        return "enemy1";
      case 1:
        return "enemy2";
      case 2:
        return "enemy3";
      case 4:
        return "boss";
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
      case 4:
        return "boss_anim";
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
      CreateExplosion(playerEntity);
      GetAudio().PlaySound("explosion");
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

    std::cout << "Spawning enemy " << enemyId << " (type "
              << static_cast<int>(enemyType) << ") at (" << position.x << ", "
              << position.y << ")" << std::endl;

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
      CreateExplosion(enemyEntity);
      GetRegistry().kill_entity(enemyEntity);
      GetAudio().PlaySound("explosion");
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

  void SpawnForce(uint16_t forceId, uint16_t ownerId, Vector2 position) {
    if (m_forces.find(forceId) != m_forces.end()) {
      return;
    }

    std::cout << "Spawning force " << forceId << " for player " << ownerId
              << " at (" << position.x << ", " << position.y << ")"
              << std::endl;

    Entity force =
        m_engine->CreateAnimatedSprite("force", position, "force_anim");

    auto& transform = GetRegistry().get_components<Transform>()[force];
    if (transform) {
      transform->scale = {2.0f, 2.0f};
    }

    m_forces[forceId] = force;
    m_entities.push_back(force);

    std::cout << "Force " << forceId << " spawned successfully" << std::endl;
  }

  void RemoveForce(uint16_t forceId) {
    auto it = m_forces.find(forceId);
    if (it != m_forces.end()) {
      Entity forceEntity = it->second;
      GetRegistry().kill_entity(forceEntity);
      m_entities.erase(
          std::remove(m_entities.begin(), m_entities.end(), forceEntity),
          m_entities.end());
      m_forces.erase(it);
      std::cout << "Removed force " << forceId << std::endl;
    }
  }

  void UpdateForcePosition(uint16_t forceId, Vector2 position) {
    auto it = m_forces.find(forceId);
    if (it != m_forces.end()) {
      Entity forceEntity = it->second;
      auto& transforms = GetRegistry().get_components<Transform>();

      if (forceEntity < transforms.size() &&
          transforms[forceEntity].has_value()) {
        transforms[forceEntity]->position = position;
      }
    }
  }

  void UpdateForces(const std::vector<GAME_STATE::ForceState>& forces,
                    float dt) {
    std::unordered_set<uint16_t> activeForceIds;

    for (const auto& forceState : forces) {
      uint16_t forceId = forceState.forceId;
      activeForceIds.insert(forceId);

      auto it = m_forces.find(forceId);
      if (it == m_forces.end()) {
        SpawnForce(forceId, forceState.ownerId,
                   {forceState.posX, forceState.posY});
      } else {
        UpdateForcePosition(forceId, {forceState.posX, forceState.posY});
      }
    }

    // Remove forces that no longer exist
    std::vector<uint16_t> toRemove;
    for (const auto& [forceId, entity] : m_forces) {
      if (activeForceIds.find(forceId) == activeForceIds.end()) {
        toRemove.push_back(forceId);
      }
    }

    for (uint16_t forceId : toRemove) {
      RemoveForce(forceId);
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
    GetAudio().PlaySound("shoot");
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
    auto& playerComponents = GetRegistry().get_components<PlayerEntity>();

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
          m_healthText->SetText(
              "Health: " + std::to_string(static_cast<int>(playerState.hp)));
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
      if (activePlayerIds.find(m_localPlayerId) == activePlayerIds.end() &&
          m_isAlive) {
        CreateExplosion(m_localPlayer);
        GetAudio().PlaySound("explosion");
        m_healthText->SetText("Health: 0");
        GetRegistry().kill_entity(m_localPlayer);
        m_entities.erase(
            std::remove(m_entities.begin(), m_entities.end(), m_localPlayer),
            m_entities.end());
        m_isAlive = false;
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
        m_nextWave = false;
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
    if (!m_nextWave && activeEnemyIds.size() == 0) {
      m_nextWave = true;
      m_wave += 1;
      if (m_wave == 5) {
        m_wave = 0;
        m_level += 1;
      }
      m_levelText->SetText("Level: " + std::to_string(m_level) +
                           " Wave: " + std::to_string(m_wave));
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

    if (e.type == EventType::GAME_END) ChangeScene("gameover");
    std::visit(
        [&](auto&& payload) {
          using T = std::decay_t<decltype(payload)>;

          if constexpr (std::is_same_v<T, GAME_STATE>) {
            UpdatePlayers(payload.players, dt);
            UpdateEnemies(payload.enemies, dt);
            UpdateProjectiles(payload.projectiles, dt);
            UpdateForces(payload.forces, dt);
          }
        },
        e.data);
  }

  void CreateExplosion(Entity entity) {
    auto& transform = GetRegistry().get_components<Transform>()[entity];

    Vector2 pos = transform->position;
    Entity explosion =
        m_engine->CreateAnimatedSprite("explosion", pos, "explode_anim");

    m_explosions[explosion] = 0.6f;
  }

  void RemoveExplosions(float dt) {
    for (auto it = m_explosions.begin(); it != m_explosions.end();) {
      it->second -= dt;
      if (it->second <= 0.0f) {
        GetRegistry().kill_entity(Entity(it->first));
        it = m_explosions.erase(it);
      } else {
        ++it;
      }
    }
  }
};
