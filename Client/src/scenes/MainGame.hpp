#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Player/PlayerEntity.hpp"
#include "inputs/InputSystem.hpp"
#include "network/DataMask.hpp"
#include "scene/SceneManager.hpp"
#include "settings/MultiplayerSkinManager.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "ui/UIManager.hpp"
#include "ui/UISolidColor.hpp"
#include "ui/UIText.hpp"

class MyGameScene : public Scene {
 private:
  Vector2 m_lastServerPosition;
  float m_positionDiff = 50.0f;
  std::unordered_map<uint16_t, GAME_STATE::PlayerState> m_playerStates;
  std::unordered_map<uint16_t, GAME_STATE::EnemyState> m_enemyStates;
  std::unordered_map<uint16_t, GAME_STATE::ProjectileState> m_projectileStates;
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
  bool m_isSpectator = false;
  uint32_t m_level;
  uint32_t m_wave;
  bool m_nextWave;
  UIText* m_scoreText;
  UIText* m_healthText;
  UIText* m_levelText;
  UIText* m_spectatorText;

  MultiplayerSkinManager m_skinManager;
  std::unordered_map<uint16_t, std::vector<Entity>> m_compositeEnemies;

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

      m_localPlayerId = GetSceneData().Get<uint16_t>("playerId", 0);
      m_isSpectator = GetSceneData().Get<bool>("isSpectator", false);

      if (!m_isSpectator) {
        PlayerSettings& settings = m_engine->GetPlayerSettings();
        m_localPlayerId = GetSceneData().Get<uint16_t>("playerId", 0);

        std::string selectedSkinAnim = settings.GetSelectedSkinAnimation();
        m_skinManager.SetLocalPlayerSkin(settings.GetSelectedSkin(),
                                         m_localPlayerId);

        std::cout << "Creating local player (ID: " << m_localPlayerId
                  << ") with skin: " << selectedSkinAnim << std::endl;

        float posX = GetSceneData().Get<float>("posX", 0);
        float posY = GetSceneData().Get<float>("posY", 0);

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

      } else {
        std::cout << "[SPECTATOR MODE] No local entity created." << std::endl;
        m_isAlive = false;
      }

      m_spectatorText = GetUI().AddElement<UIText>(10, 10, "SPECTATOR", "", 50);
      m_spectatorText->SetLayer(100);

      m_scoreText = GetUI().AddElement<UIText>(10, 10, "Score: 0", "", 20);
      m_scoreText->SetLayer(100);

      m_healthText = GetUI().AddElement<UIText>(10, 40, "Health: 100", "", 20);
      m_healthText->SetLayer(100);

      m_levelText = GetUI().AddElement<UIText>(10, 60, "Level: 0", "", 20);
      m_levelText->SetLayer(100);

      if (m_isSpectator) {
        m_spectatorText->SetVisible(true);
        m_scoreText->SetVisible(false);
        m_healthText->SetVisible(false);
        m_levelText->SetVisible(false);
      } else {
        m_spectatorText->SetVisible(false);
        m_scoreText->SetVisible(true);
        m_healthText->SetVisible(true);
        m_levelText->SetVisible(true);
      }

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
    m_isSpectator = false;
    m_firstState = false;
    GetUI().Clear();
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
    if (!textures.GetTexture("enemy4")) {
      textures.LoadTexture("enemy4", "../Client/assets/enemy4.png");
    }
    if (!textures.GetTexture("boom3")) {
      textures.LoadTexture("boom3", "../Client/assets/boom.png");
    }

    if (!textures.GetTexture("projectile_player")) {
      textures.LoadTexture("projectile_player",
                           "../Client/assets/blueShoot.png");
    }
    if (!textures.GetTexture("charged_projectile_palyer")) {
      textures.LoadTexture("charged_projectil_palyer",
                           "../Client/assets/charged.png");
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
    if (!textures.GetTexture("boss2")) {
      textures.LoadTexture("boss2", "../Client/assets/boss2.png");
    }
    if (!textures.GetTexture("boss3")) {
      textures.LoadTexture("boss3", "../Client/assets/boss3.png");
    }
    if (!textures.GetTexture("head_boss")) {
      textures.LoadTexture("head_boss", "../Client/assets/boss_head.png");
    }
    if (!textures.GetTexture("enemy5")) {
      textures.LoadTexture("enemy5", "../Client/assets/enemy5.png");
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
                                {{189, 1711, 154, 203}, 0.5f},
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

    animations.CreateAnimation("enemy4_anim", "enemy4",
                               {{{0, 0, 55, 94}, 0.2f},
                                {{55, 0, 55, 94}, 0.2f},
                                {{110, 0, 55, 94}, 0.2f}},
                               true);

    animations.CreateAnimation("enemy5_anim", "enemy5",
                               {{{0, 0, 33, 29}, 0.1f},
                                {{33, 0, 33, 29}, 0.1f},
                                {{66, 0, 34, 29}, 0.1f}},
                               true);

    animations.CreateAnimation("projectile_player_anim", "projectile_player",
                               {{{1, 0, 17, 5}, 0.1f}, {{19, 0, 17, 5}, 0.1f}},
                               true);

    animations.CreateAnimation("projectile_charged", "projectile_player",
                               {{{1, 0, 17, 5}, 0.1f}, {{19, 0, 17, 5}, 0.1f}},
                               true);

    animations.CreateAnimation(
        "projectile_enemy_anim", "projectile_enemy",
        {{{0, 0, 12, 12}, 0.1f}, {{12, 0, 12, 12}, 0.1f}}, true);

    animations.CreateAnimation("force_anim", "force",
                               {
                                   {{0, 0, 30, 25}, 0.08f},
                                   {{30, 0, 30, 25}, 0.08f},
                                   {{60, 0, 30, 25}, 0.08f},
                                   {{90, 0, 30, 25}, 0.08f},
                                   {{120, 0, 30, 25}, 0.08f},
                                   {{150, 0, 30, 25}, 0.08f},
                                   {{180, 0, 30, 25}, 0.08f},
                                   {{210, 0, 30, 25}, 0.08f},
                                   {{240, 0, 30, 25}, 0.08f},
                                   {{270, 0, 30, 25}, 0.08f},
                                   {{300, 0, 30, 25}, 0.08f},
                                   {{330, 0, 30, 25}, 0.08f},
                               },
                               true);

    animations.CreateAnimation("boss_serpent_head_up", "head_boss",
                               {
                                   {{0, 0, 34, 32}, 0.1f},
                                   {{34, 0, 34, 32}, 0.1f},
                                   {{68, 0, 34, 32}, 0.1f},
                               },
                               true);

    animations.CreateAnimation("boss_serpent_body_anim", "boss2",
                               {
                                   {{0, 0, 34, 29}, 0.1f},
                                   {{34, 0, 34, 29}, 0.1f},
                                   {{68, 0, 34, 29}, 0.1f},
                                   {{102, 0, 34, 29}, 0.1f},
                                   {{136, 0, 34, 29}, 0.1f},
                                   {{170, 0, 34, 29}, 0.1f},
                                   {{204, 0, 34, 29}, 0.1f},
                               },
                               true);

    animations.CreateAnimation("boss3_part", "boom3",
                               {{{0, 0, 587, 180}, 1.0f}}, true);

    animations.CreateAnimation("boom_anim", "boom_sprite",
                               {
                                   {{0 * 34, 0, 34, 29}, 0.1f},  // frame 1
                                   {{1 * 34, 0, 34, 29}, 0.1f},  // frame 2
                                   {{2 * 34, 0, 34, 29}, 0.1f},  // frame 3
                                   {{3 * 34, 0, 34, 29}, 0.1f},  // frame 4
                                   {{4 * 34, 0, 34, 29}, 0.1f},  // frame 5
                                   {{5 * 34, 0, 34, 29}, 0.1f},  // frame 6
                                   {{6 * 34, 0, 34, 29}, 0.1f},  // frame 7
                                   {{7 * 34, 0, 34, 29}, 0.1f},  // frame 8
                                   {{8 * 34, 0, 34, 29}, 0.1f},  // frame 9
                               },
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
      case 3:
        return "enemy4";
      case 4:
        return "enemy5";

      // BossParts (90+)
      case 90:
        return "boss";  // default part
      case 91:
        return "boss2";  // serpent body
      case 92:
        return "boss3";  // turret

      // Bosses (100+)
      case 100:
        return "boss";  // FinalBoss
      case 101:
        return "head_boss";  // Gomander_snake (tête)
      case 102:
        return "boss";  // BigShip
      case 103:
        return "boss";  // BydoEye
      case 104:
        return "boss3";  // Bydo_Battleship

      default:
        return "enemy1";
    }
  }

  std::string GetEnemyAnimation(uint8_t enemyType) const {
    switch (enemyType) {
      // Enemies normaux
      case 0:
        return "enemy1_anim";
      case 1:
        return "enemy2_anim";
      case 2:
        return "enemy3_anim";
      case 3:
        return "enemy4_anim";
      case 4:
        return "enemy5_anim";

      // BossParts (90+)
      case 90:
        return "boss_anim";
      case 91:
        return "boss_serpent_body_anim";  // ← Corps du serpent
      case 92:
        return "boss_anim";  // turret

      // Bosses (100+)
      case 100:
        return "boss_anim";
      case 101:
        return "boss_serpent_head_up";
      case 102:
        return "boss_anim";
      case 103:
        return "boss_anim";
      case 104:
        return "boss3_anim";

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
      case 2:
        return "charged_projectile_player";
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
      case 2:
        return "projectile_charged";
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

  void UpdateEnemyPosition(uint16_t enemyId, Vector2 targetPos) {
    auto it = m_enemies.find(enemyId);
    if (it != m_enemies.end()) {
      Entity enemyEntity = it->second;
      auto& transforms = GetRegistry().get_components<Transform>();

      if (enemyEntity < transforms.size() &&
          transforms[enemyEntity].has_value()) {
        Vector2& currentPos = transforms[enemyEntity]->position;
        currentPos.x += (targetPos.x - currentPos.x) * 0.5f;
        currentPos.y += (targetPos.y - currentPos.y) * 0.5f;
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
      transform->scale = {1.3f, 1.3f};
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

  void UpdateForces(const std::vector<ForceState>& forces, float dt) {
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
                       Vector2 position, uint8_t chargeLevel = 0) {
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
      if (projectileType == 2) {
        transform->scale = {3.0f, 3.0f};  // Gros projectile chargé
      } else {
        transform->scale = {1.5f, 1.5f};  // Projectile normal
      }
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

    for (const auto& deltaState : playerStates) {
      uint16_t playerId = deltaState.playerId;

      if (deltaState.mask & M_DELETE) {
        if (playerId == m_localPlayerId && !m_isSpectator) {
          m_isAlive = false;
          m_isSpectator = true;
          m_spectatorText->SetVisible(true);
          CreateExplosion(m_localPlayer);
          GetAudio().PlaySound("explosion");
          m_healthText->SetVisible(false);
          m_scoreText->SetVisible(false);
          m_levelText->SetVisible(false);
          if (GetRegistry().is_entity_valid(m_localPlayer)) {
            GetRegistry().kill_entity(m_localPlayer);
          }
        } else {
          RemoveOtherPlayer(playerId);
        }
        m_playerStates.erase(playerId);
        continue;
      }

      auto& fullState = m_playerStates[playerId];

      if (deltaState.mask & M_POS_X) fullState.posX = deltaState.posX;
      if (deltaState.mask & M_POS_Y) fullState.posY = deltaState.posY;
      if (deltaState.mask & M_HP) fullState.hp = deltaState.hp;
      if (deltaState.mask & M_SHIELD) fullState.shield = deltaState.shield;
      if (deltaState.mask & M_WEAPON) fullState.weapon = deltaState.weapon;
      if (deltaState.mask & M_STATE) fullState.state = deltaState.state;
      if (deltaState.mask & M_SPRITE) fullState.sprite = deltaState.sprite;
      fullState.playerId = playerId;

      if (!m_isSpectator && playerId == m_localPlayerId) {
        if (m_localPlayer < transforms.size() &&
            transforms[m_localPlayer].has_value()) {
          auto& pos = transforms[m_localPlayer]->position;

          float minX = 30.0f;
          float maxX = 770.0f;
          float minY = 30.0f;
          float maxY = 570.0f;

          if (pos.x < minX) pos.x = minX;
          if (pos.x > maxX) pos.x = maxX;
          if (pos.y < minY) pos.y = minY;
          if (pos.y > maxY) pos.y = maxY;

          Vector2 serverPos = {fullState.posX, fullState.posY};
          Vector2 clientPos = pos;

          float dx = serverPos.x - clientPos.x;
          float dy = serverPos.y - clientPos.y;
          float distance = std::sqrt(dx * dx + dy * dy);

          if (distance > m_positionDiff) {
            pos = serverPos;
          } else if (distance > 1.0f) {
            pos.x += dx * 0.3f;
            pos.y += dy * 0.3f;
          }
        }

        if (m_localPlayer < playerComponents.size() &&
            playerComponents[m_localPlayer].has_value()) {
          playerComponents[m_localPlayer]->current =
              static_cast<int>(fullState.hp);
          m_healthText->SetText("Health: " +

                                std::to_string(static_cast<int>(fullState.hp)));
        }
        if (deltaState.mask & M_HP) {
          if (fullState.hp <= 0 && m_isAlive) {
            m_isAlive = false;
            m_isSpectator = true;
            m_spectatorText->SetVisible(true);
            CreateExplosion(m_localPlayer);
            GetAudio().PlaySound("explosion");
            m_healthText->SetVisible(false);
            m_scoreText->SetVisible(false);
            m_levelText->SetVisible(false);
            if (GetRegistry().is_entity_valid(m_localPlayer)) {
              GetRegistry().kill_entity(m_localPlayer);
            }
          }
        }
      } else {
        auto it = m_otherPlayers.find(playerId);
        if (it == m_otherPlayers.end()) {
          SpawnOtherPlayer(playerId, {fullState.posX, fullState.posY});
        } else {
          if (fullState.hp <= 0) {
            RemoveOtherPlayer(playerId);
          } else {
            UpdateOtherPlayerPosition(playerId,
                                      {fullState.posX, fullState.posY});
          }
        }
      }
    }
    m_firstState = true;
  }

  void UpdateEnemies(const std::vector<GAME_STATE::EnemyState>& enemies,
                     float dt) {
    for (const auto& deltaState : enemies) {
      uint16_t enemyId = deltaState.enemyId;

      if (deltaState.mask & M_DELETE) {
        RemoveEnemy(enemyId);
        m_enemyStates.erase(enemyId);
        continue;
      }

      auto& fullState = m_enemyStates[enemyId];

      if (deltaState.mask & M_POS_X) fullState.posX = deltaState.posX;
      if (deltaState.mask & M_POS_Y) fullState.posY = deltaState.posY;
      if (deltaState.mask & M_HP) fullState.hp = deltaState.hp;
      if (deltaState.mask & M_TYPE) fullState.enemyType = deltaState.enemyType;
      if (deltaState.mask & M_STATE) fullState.state = deltaState.state;
      if (deltaState.mask & M_DIR) fullState.direction = deltaState.direction;
      fullState.enemyId = enemyId;

      if (fullState.hp <= 0) {
        RemoveEnemy(enemyId);
        m_enemyStates.erase(enemyId);
        continue;
      }

      auto it = m_enemies.find(enemyId);
      if (it == m_enemies.end()) {
        m_nextWave = false;
        SpawnEnemy(enemyId, fullState.enemyType,
                   {fullState.posX, fullState.posY});
      } else {
        UpdateEnemyPosition(enemyId, {fullState.posX, fullState.posY});
      }
    }

    if (!m_nextWave && m_enemies.empty()) {
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
    for (const auto& deltaState : projectiles) {
      uint16_t projectileId = deltaState.projectileId;

      if (deltaState.mask & M_DELETE) {
        RemoveProjectile(projectileId);
        m_projectileStates.erase(projectileId);
        continue;
      }

      auto& fullState = m_projectileStates[projectileId];

      if (deltaState.mask & M_POS_X) fullState.posX = deltaState.posX;
      if (deltaState.mask & M_POS_Y) fullState.posY = deltaState.posY;
      if (deltaState.mask & M_DAMAGE) fullState.damage = deltaState.damage;
      if (deltaState.mask & M_TYPE) fullState.type = deltaState.type;
      if (deltaState.mask & M_OWNER) fullState.ownerId = deltaState.ownerId;
      fullState.projectileId = projectileId;

      if (fullState.damage == 0) {
        RemoveProjectile(projectileId);
        m_projectileStates.erase(projectileId);
        continue;
      }

      auto it = m_projectiles.find(projectileId);
      if (it == m_projectiles.end()) {
        SpawnProjectile(projectileId, fullState.type,
                        {fullState.posX, fullState.posY});
      } else {
        UpdateProjectilePosition(projectileId,
                                 {fullState.posX, fullState.posY});
      }
    }
  }

  void GetEvents(float dt) {
    while (true) {
      Event e = GetNetwork().PopEvent();
      if (e.type == EventType::UNKNOWN) break;
      if (e.type == EventType::GAME_END) {
        ChangeScene("gameover");
        return;
      }

      // DEBUG : Affiche le type d'événement reçu
    std::cout << "[DEBUG] Event type: " << static_cast<int>(e.type)
              << std::endl;

    if (e.type == EventType::GAME_END) ChangeScene("gameover");

    std::visit(
          [&](auto&& payload) {
            using T = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<T, GAME_STATE>) {
              std::cout << "[CLIENT] GAME_STATE recu!" << std::endl;
            UpdatePlayers(payload.players, dt);
              UpdateEnemies(payload.enemies, dt);
              UpdateProjectiles(payload.projectiles, dt);
            } else if constexpr (std::is_same_v<T, FORCE_STATE>) {
            // DEBUG
            std::cout << "[DEBUG] FORCE_STATE recu! forceId=" << payload.forceId
                      << " pos=(" << payload.posX << ", " << payload.posY << ")"
                      << std::endl;

            auto it = m_forces.find(payload.forceId);
            if (it == m_forces.end()) {
              SpawnForce(payload.forceId, payload.ownerId,
                         {payload.posX, payload.posY});
            } else {
              UpdateForcePosition(payload.forceId,
                                  {payload.posX, payload.posY});
            }
          }
          },
          e.data);
    }
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

  void RemoveCompositeEnemy(uint16_t enemyId) {
    auto it = m_compositeEnemies.find(enemyId);
    if (it != m_compositeEnemies.end()) {
      auto& segments = it->second;
      for (auto& segment : segments) {
        CreateExplosion(segment);  // optionnel, explosion pour chaque segment
        GetRegistry().kill_entity(segment);
        m_entities.erase(
            std::remove(m_entities.begin(), m_entities.end(), segment),
            m_entities.end());
      }
      m_compositeEnemies.erase(it);
      std::cout << "Removed composite enemy " << enemyId << std::endl;
    }
  }

  void SpawnSegmentedEnemy(uint16_t enemyId, uint8_t enemyType,
                           Vector2 position, uint8_t segments) {
    if (m_enemies.find(enemyId) != m_enemies.end()) {
      return;
    }

    std::string texture = GetEnemyTexture(enemyType);
    std::string animation = GetEnemyAnimation(enemyType);

    std::cout << "Spawning segmented enemy " << enemyId << " (type "
              << static_cast<int>(enemyType) << ") with "
              << static_cast<int>(segments) << " segments at (" << position.x
              << ", " << position.y << ")" << std::endl;

    std::vector<Entity> segmentEntities;

    for (int i = 0; i < segments; ++i) {
      Vector2 segmentPos = position;
      segmentPos.x -= i * 30;  // espace entre les segments

      Entity segment =
          m_engine->CreateAnimatedSprite(texture, segmentPos, animation);

      auto& transform = GetRegistry().get_components<Transform>()[segment];
      if (transform) {
        transform->scale = {2.0f, 2.0f};
      }

      segmentEntities.push_back(segment);
      m_entities.push_back(segment);
    }
    m_compositeEnemies[enemyId] = segmentEntities;
    std::cout << "Segmented enemy " << enemyId << " spawned successfully"
              << std::endl;
  }
  void UpdateCompositeEnemyPosition(uint16_t enemyId, Vector2 position,
                                    float segmentSpacing = 30.0f) {
    auto it = m_compositeEnemies.find(enemyId);
    if (it != m_compositeEnemies.end()) {
      auto& segments = it->second;
      for (size_t i = 0; i < segments.size(); ++i) {
        Entity segment = segments[i];
        auto& transforms = GetRegistry().get_components<Transform>();
        if (segment < transforms.size() && transforms[segment].has_value()) {
          transforms[segment]->position = {
              position.x - static_cast<float>(i) * segmentSpacing, position.y};
        }
      }
    }
  }
};
