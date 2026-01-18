#include "scenes/RtypeScene.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "Collision/Collision.hpp"
#include "Movement/Movement.hpp"
#include "ecs/Zipper.hpp"
#include "network/DataMask.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/ChargedShoot.hpp"
#include "systems/ForceCtrl.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"

RtypeScene::RtypeScene() { m_name = "rtype server scene"; }

void RtypeScene::OnEnter() {
  SceneData& data = GetSceneData();
  auto players_list =
      data.Get<std::vector<std::tuple<uint16_t, bool, std::string>>>(
          "players_list");
  // Physics components
  GetRegistry().register_component<Transform>();
  GetRegistry().register_component<RigidBody>();
  GetRegistry().register_component<BoxCollider>();
  GetRegistry().register_component<TileMap>();

  // Player components
  GetRegistry().register_component<PlayerEntity>();
  GetRegistry().register_component<InputState>();

  // Enemy / Gameplay
  GetRegistry().register_component<Enemy>();
  GetRegistry().register_component<Boss>();
  GetRegistry().register_component<Items>();
  GetRegistry().register_component<Collision>();
  GetRegistry().register_component<EnemySpawning>();

  // Weapon & Projectile components
  GetRegistry().register_component<Weapon>();
  GetRegistry().register_component<Projectile>();
  GetRegistry().register_component<LevelComponent>();
  GetRegistry().register_component<BossPart>();
  GetRegistry().register_component<Force>();

  currentMap = generateSimpleMap(0, 800, 600);
  mapEntity =
      createMapEntity(GetRegistry(), currentMap.width, currentMap.height,
                      currentMap.scrollSpeed, currentMap.tiles);
  mapSent = false;

  MapData mapData;
  mapData.width = currentMap.width;
  mapData.height = currentMap.height;
  mapData.scrollSpeed = currentMap.scrollSpeed;
  mapData.tiles = currentMap.tiles;
  data.Set("send_map", mapData);

  mapSent = true;
  std::cout << "[StartGame] Map created!" << std::endl;

  for (auto& [playerId, ready, _] : players_list) {
    float posY = 200.f + (m_players.size() % 4) * 100.f;
    Entity player = createPlayer(GetRegistry(), {200, posY}, playerId);
    Entity forceEntity = createForce(GetRegistry(), player, {200, posY});
    m_players[playerId] = player;
  }

  levelsData = createLevels();
  currentLevelIndex = 0;
  waitingForNextLevel = false;
  levelTransitionTimer = 0.0f;
  currentLevelEntity =
      createLevelEntity(GetRegistry(), levelsData[currentLevelIndex]);
  std::cout << "[StartGame] Level " << currentLevelIndex + 1 << " created"
            << std::endl;

  std::cout << "Lobby full! Starting the game!!!!\n";
  //   lobby_list.gameThread =
  //       std::thread(&ServerGame::GameLoop, this, std::ref(lobby_list));
}

void RtypeScene::OnExit() { m_players.clear(); }

void RtypeScene::Update(float deltaTime) {
  ReceivePlayerInputs();
  UpdateGameState(deltaTime);
  BuildCurrentState();
}

void RtypeScene::ReceivePlayerInputs() {
  SceneData& data = GetSceneData();

  if (data.Has("last_input_event")) {
    Event event = data.Get<Event>("last_input_event");
    uint16_t playerId = data.Get<uint16_t>("last_input_player");

    if (event.type == EventType::PLAYER_INPUT) {
      const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);

      auto& players = GetRegistry().get_components<PlayerEntity>();
      auto& states = GetRegistry().get_components<InputState>();

      for (auto&& [player, state] : Zipper(players, states)) {
        if (player.player_id != playerId) continue;

        state.moveLeft = input.left;
        state.moveRight = input.right;
        state.moveDown = input.down;
        state.moveUp = input.up;
        state.action1 = input.fire;
        break;
      }
    }

    data.Remove("last_input_event");
    data.Remove("last_input_player");
  }
}

void RtypeScene::UpdateGameState(float deltaTime) {
  SceneData& data = GetSceneData();
  auto& transforms = GetRegistry().get_components<Transform>();
  auto& rigidbodies = GetRegistry().get_components<RigidBody>();
  auto& players = GetRegistry().get_components<PlayerEntity>();
  auto& enemies = GetRegistry().get_components<Enemy>();
  auto& bosses = GetRegistry().get_components<Boss>();
  auto& colliders = GetRegistry().get_components<BoxCollider>();
  auto& projectiles = GetRegistry().get_components<Projectile>();
  auto& weapons = GetRegistry().get_components<Weapon>();
  auto& forces = GetRegistry().get_components<Force>();
  auto& states = GetRegistry().get_components<InputState>();

  for (auto&& [player] : Zipper(players)) {
    if (player.invtimer > 0.0f) {
      player.invtimer -= deltaTime;
      if (player.invtimer < 0.0f) player.invtimer = 0.0f;
    }
  }

  auto& levelsDbg = GetRegistry().get_components<LevelComponent>();
  int countLevels = 0;
  for (auto& lvl : levelsDbg) {
    if (lvl.has_value()) {
      countLevels++;
    }
  }
  int enemyCount = 0;
  for (auto& en : enemies) {
    if (en.has_value()) enemyCount++;
  }
  int bossCount = 0;
  for (auto& bo : bosses) {
    if (bo.has_value()) bossCount++;
  }

  if (waitingForNextLevel) {
    levelTransitionTimer += deltaTime;
    if (levelTransitionTimer >= TIME_BETWEEN_LEVELS) {
      levelTransitionTimer = 0.0f;
      waitingForNextLevel = false;
      currentLevelIndex++;

      if (currentLevelIndex >= static_cast<int>(levelsData.size())) {
        data.Set("game_ended", true);
        data.Set("victory", true);
        std::cout << "[Game] VICTORY! All levels completed!" << std::endl;
        return;
      }

      currentLevelEntity =
          createLevelEntity(GetRegistry(), levelsData[currentLevelIndex]);
      std::cout << "[Game] Level " << (currentLevelIndex + 1)
                << " created (Entity "
                << static_cast<size_t>(currentLevelEntity) << ")" << std::endl;
    }
  } else {
    bool levelFinished =
        update_level_system(GetRegistry(), deltaTime, currentLevelIndex);

    if (levelFinished) {
      std::vector<Entity> toKill;

      auto& enemiesCleanup = GetRegistry().get_components<Enemy>();
      for (size_t i = 0; i < enemiesCleanup.size(); ++i) {
        if (enemiesCleanup[i].has_value()) {
          toKill.push_back(GetRegistry().entity_from_index(i));
        }
      }

      auto& bossesCleanup = GetRegistry().get_components<Boss>();
      for (size_t i = 0; i < bossesCleanup.size(); ++i) {
        if (bossesCleanup[i].has_value()) {
          toKill.push_back(GetRegistry().entity_from_index(i));
        }
      }

      auto& bossPartsCleanup = GetRegistry().get_components<BossPart>();
      for (size_t i = 0; i < bossPartsCleanup.size(); ++i) {
        if (bossPartsCleanup[i].has_value()) {
          toKill.push_back(GetRegistry().entity_from_index(i));
        }
      }

      for (Entity e : toKill) {
        GetRegistry().kill_entity(e);
      }

      std::cout << "[DEBUG] Killing level entity: "
                << static_cast<size_t>(currentLevelEntity) << std::endl;
      GetRegistry().kill_entity(currentLevelEntity);

      waitingForNextLevel = true;
      levelTransitionTimer = 0.0f;
      std::cout << "[Game] Level " << (currentLevelIndex + 1)
                << " finished! Waiting " << TIME_BETWEEN_LEVELS << " seconds..."
                << std::endl;
    }
  }
  player_movement_system(GetRegistry());
  charged_shoot_system(GetRegistry(), deltaTime);
  physics_movement_system(GetRegistry(), transforms, rigidbodies, deltaTime,
                          {0, 0});
  enemy_movement_system(GetRegistry(), transforms, rigidbodies, enemies,
                        players, deltaTime);
  boss_movement_system(GetRegistry(), transforms, rigidbodies, bosses,
                       deltaTime);
  boss_part_system(GetRegistry(), deltaTime);
  weapon_cooldown_system(GetRegistry(), weapons, deltaTime);
  weapon_reload_system(GetRegistry(), weapons, deltaTime);
  force_control_system(GetRegistry(), forces, states, transforms);
  force_movement_system(GetRegistry(), transforms, rigidbodies, forces, players,
                        deltaTime);
  weapon_firing_system(
      GetRegistry(), weapons, transforms,
      [this](size_t entityId) -> bool {
        auto& playerEntity = GetRegistry().get_components<PlayerEntity>();
        if (entityId < playerEntity.size() &&
            playerEntity[entityId].has_value()) {
          auto& state = GetRegistry().get_components<InputState>()[entityId];
          return state->action1;
        }
        return false;
      },
      deltaTime);

  projectile_collision_system(GetRegistry(), transforms, colliders,
                              projectiles);
  projectile_lifetime_system(GetRegistry(), projectiles, deltaTime);
  gamePlay_Collision_system(GetRegistry(), transforms, colliders, players,
                            enemies, bosses);
  bounds_check_system(GetRegistry(), transforms, colliders, rigidbodies);
  force_collision_system(GetRegistry(), transforms, colliders, forces, enemies,
                         bosses, GetRegistry().get_components<BossPart>(),
                         projectiles);
}

void RtypeScene::BuildCurrentState() {
  auto& transforms = GetRegistry().get_components<Transform>();
  auto& players = GetRegistry().get_components<PlayerEntity>();
  auto& enemies = GetRegistry().get_components<Enemy>();
  auto& bosses = GetRegistry().get_components<Boss>();
  auto& projectiles = GetRegistry().get_components<Projectile>();
  auto& bosspart = GetRegistry().get_components<BossPart>();
  auto& forcesArr = GetRegistry().get_components<Force>();

  GameState gs;

  for (auto&& [idx, player, transform] : IndexedZipper(players, transforms)) {
    Entity e = GetRegistry().entity_from_index(idx);
    if (!GetRegistry().is_entity_valid(e)) continue;

    PlayerState ps;
    ps.playerId = player.player_id;
    ps.posX = transform.position.x;
    ps.posY = transform.position.y;
    ps.hp = static_cast<uint8_t>(player.current);
    ps.shield = 0;
    ps.weapon = static_cast<uint8_t>(0);
    ps.state = player.isAlive ? 1 : 0;
    ps.sprite = 0;
    ps.score = player.score;

    ps.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_SHIELD | M_WEAPON |
              M_SPRITE | M_SCORE;

    gs.players.push_back(ps);
  }

  for (auto&& [idx, enemy, transform] : IndexedZipper(enemies, transforms)) {
    Entity e = GetRegistry().entity_from_index(idx);
    if (!GetRegistry().is_entity_valid(e)) continue;

    EnemyState es;
    es.enemyId = static_cast<uint16_t>(idx);
    es.enemyType = static_cast<uint8_t>(enemy.type);
    es.posX = transform.position.x;
    es.posY = transform.position.y;
    es.hp = static_cast<uint8_t>(enemy.current);
    es.state = 1;
    es.direction = 0;

    es.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_TYPE | M_DIR;

    gs.enemies.push_back(es);
  }

  for (auto&& [idx, segment, transform] : IndexedZipper(bosspart, transforms)) {
    Entity e = GetRegistry().entity_from_index(idx);
    if (!GetRegistry().is_entity_valid(e)) continue;

    EnemyState es;
    es.enemyId = static_cast<uint16_t>(idx);
    es.enemyType = 90 + segment.partType;
    es.posX = transform.position.x;
    es.posY = transform.position.y;

    es.hp = 1;
    es.state = 1;
    es.direction = 0;
    es.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_TYPE;
    gs.enemies.push_back(es);
  }

  for (auto&& [idx, boss, transform] : IndexedZipper(bosses, transforms)) {
    Entity e = GetRegistry().entity_from_index(idx);
    if (!GetRegistry().is_entity_valid(e)) continue;

    EnemyState bs;
    bs.enemyId = static_cast<uint16_t>(idx);
    bs.enemyType = static_cast<uint8_t>(boss.type) + 100;
    bs.posX = transform.position.x;
    bs.posY = transform.position.y;
    bs.hp = static_cast<uint8_t>(boss.current);
    bs.state = 1;
    bs.direction = 0;
    bs.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_TYPE;
    gs.enemies.push_back(bs);
  }

  for (auto&& [idx, proj, transform] : IndexedZipper(projectiles, transforms)) {
    Entity e = GetRegistry().entity_from_index(idx);
    if (!GetRegistry().is_entity_valid(e)) continue;

    ProjectileState ps;
    ps.projectileId = static_cast<uint16_t>(idx);
    ps.ownerId = proj.ownerId;
    ps.type = 0;
    ps.posX = transform.position.x;
    ps.posY = transform.position.y;
    ps.velX = proj.direction.x * proj.speed;
    ps.velY = proj.direction.y * proj.speed;
    ps.damage = static_cast<uint8_t>(proj.damage);

    if (proj.chargeLevel > 0) {
      ps.type = 2;  // Tir chargé
    } else {
      ps.type = 0;  // Tir normal
    }

    ps.mask = M_POS_X | M_POS_Y | M_TYPE | M_OWNER | M_DAMAGE;

    gs.projectiles.push_back(ps);
  }

  SceneData& data = GetSceneData();
  for (auto&& [idx, force, transform] : IndexedZipper(forcesArr, transforms)) {
    if (!force.isActive) continue;

    ForceState fs;
    fs.forceId = static_cast<uint16_t>(idx);
    fs.ownerId = static_cast<uint16_t>(force.ownerPlayer);
    fs.posX = transform.position.x;
    fs.posY = transform.position.y;
    fs.state = static_cast<uint8_t>(force.state);
    data.Set("force_state", fs);
  }
  data.Set("game_state", gs);
}

void RtypeScene::HandleEvent(SDL_Event& event) {}

extern "C" {
Scene* CreateScene() { return new RtypeScene(); }
}
