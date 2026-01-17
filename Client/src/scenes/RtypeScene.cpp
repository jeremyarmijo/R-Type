#include "scenes/RtypeScene.hpp"
#include "systems/LevelSystem.hpp"
#include "ecs/Zipper.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "Movement/Movement.hpp"
#include "Collision/Collision.hpp"

RtypeScene::RtypeScene() { m_name = "rtype server scene"; }

void RtypeScene::OnEnter() {
  SceneData& data = GetSceneData();
  auto players_list = data.Get<std::vector<std::tuple<uint16_t, bool, std::string>>>("players_list");
  // Physics components
  GetRegistry().register_component<Transform>();
  GetRegistry().register_component<RigidBody>();
  GetRegistry().register_component<BoxCollider>();

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

  for (auto& [playerId, ready, _] : players_list) {
    float posY = 200.f + (m_players.size() % 4) * 100.f;
    Entity player = createPlayer(GetRegistry(), {200, posY}, playerId);
    m_players[playerId] = player;
  }

  levelsData = createLevels();
  currentLevelIndex = 0;
  waitingForNextLevel = false;
  levelTransitionTimer = 0.0f;
  currentLevelEntity = createLevelEntity(
      GetRegistry(), levelsData[currentLevelIndex]);
  std::cout << "[StartGame] Level " << currentLevelIndex + 1
            << " created" << std::endl;

  std::cout << "Lobby full! Starting the game!!!!\n";
//   lobby_list.gameThread =
//       std::thread(&ServerGame::GameLoop, this, std::ref(lobby_list));
}

void RtypeScene::OnExit() {
    m_players.clear();
}

void RtypeScene::Update(float deltaTime) {
    ReceivePlayerInputs();
    UpdateGameState(deltaTime);
    SendWorldStateToClients();
    // CheckGameEnded(lobby);
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

  for (auto&& [player] : Zipper(players)) {
    if (player.invtimer > 0.0f) {
      player.invtimer -= deltaTime;
      if (player.invtimer < 0.0f) player.invtimer = 0.0f;
    }
  }

  for (size_t i = 0; i < bosses.size(); ++i) {
    if (bosses[i].has_value()) {
      if (i < transforms.size() && transforms[i].has_value()) {
        std::cout << "[DEBUG] Boss " << i << " at position ("
                  << transforms[i]->position.x << ", "
                  << transforms[i]->position.y << ") HP=" << bosses[i]->current
                  << std::endl;
      } else {
        std::cout << "[DEBUG] Boss " << i << " has NO TRANSFORM!" << std::endl;
      }
    }
  }

  std::cout << "[DEBUG] currentLevelIndex=" << currentLevelIndex
            << " waitingForNextLevel=" << waitingForNextLevel
            << " levelTransitionTimer=" << levelTransitionTimer
            << std::endl;

  auto& levelsDbg = GetRegistry().get_components<LevelComponent>();
  int countLevels = 0;
  for (auto& lvl : levelsDbg) {
    if (lvl.has_value()) {
      countLevels++;
      std::cout << "[DEBUG] LevelComponent found: currentWave="
                << lvl->currentWave << " finishedLevel=" << lvl->finishedLevel
                << " initialized=" << lvl->initialized << std::endl;
    }
  }
  std::cout << "[DEBUG] LevelComponents in registry: " << countLevels
            << std::endl;

  int enemyCount = 0;
  for (auto& en : enemies) {
    if (en.has_value()) enemyCount++;
  }
  int bossCount = 0;
  for (auto& bo : bosses) {
    if (bo.has_value()) bossCount++;
  }
  std::cout << "[DEBUG] Enemies alive: " << enemyCount
            << " Bosses alive: " << bossCount << std::endl;

  if (waitingForNextLevel) {
    levelTransitionTimer += deltaTime;
    std::cout << "[DEBUG] Waiting for next level... "
              << levelTransitionTimer << "/" << TIME_BETWEEN_LEVELS
              << std::endl;

    if (levelTransitionTimer >= TIME_BETWEEN_LEVELS) {
      levelTransitionTimer = 0.0f;
      waitingForNextLevel = false;
      currentLevelIndex++;

      if (currentLevelIndex >=
          static_cast<int>(levelsData.size())) {
        data.Set("game_ended", true);
        data.Set("victory", true);
        std::cout << "[Game] VICTORY! All levels completed!" << std::endl;
        return;
      }

      currentLevelEntity = createLevelEntity(
          GetRegistry(), levelsData[currentLevelIndex]);
      std::cout << "[Game] Level " << (currentLevelIndex + 1)
                << " created (Entity "
                << static_cast<size_t>(currentLevelEntity) << ")"
                << std::endl;
    }
  } else {
    bool levelFinished =
        update_level_system(GetRegistry(), deltaTime, currentLevelIndex);
    std::cout << "[DEBUG] update_level_system returned: " << levelFinished
              << std::endl;

    if (levelFinished) {
      std::cout << "[DEBUG] Cleaning up remaining enemies and bosses..."
                << std::endl;

      std::vector<Entity> toKill;

      auto& enemiesCleanup = GetRegistry().get_components<Enemy>();
      for (size_t i = 0; i < enemiesCleanup.size(); ++i) {
        if (enemiesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking enemy at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(GetRegistry().entity_from_index(i));
        }
      }

      auto& bossesCleanup = GetRegistry().get_components<Boss>();
      for (size_t i = 0; i < bossesCleanup.size(); ++i) {
        if (bossesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking boss at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(GetRegistry().entity_from_index(i));
        }
      }
      for (Entity e : toKill) {
        std::cout << "[DEBUG] Killing leftover entity: "
                  << static_cast<size_t>(e) << std::endl;
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
  physics_movement_system(GetRegistry(), transforms, rigidbodies, deltaTime,
                          {0, 0});
  enemy_movement_system(GetRegistry(), transforms, rigidbodies, enemies,
                        players, deltaTime);
  boss_movement_system(GetRegistry(), transforms, rigidbodies, bosses,
                       deltaTime);
  boss_part_system(GetRegistry(), deltaTime);
  weapon_cooldown_system(GetRegistry(), weapons, deltaTime);
  weapon_reload_system(GetRegistry(), weapons, deltaTime);
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
}

void RtypeScene::SendWorldStateToClients() {
  SceneData& data = GetSceneData();
  auto& transforms = GetRegistry().get_components<Transform>();
  auto& players = GetRegistry().get_components<PlayerEntity>();
  auto& enemies = GetRegistry().get_components<Enemy>();
  auto& bosses = GetRegistry().get_components<Boss>();
  auto& projectiles = GetRegistry().get_components<Projectile>();
  auto& bosspart = GetRegistry().get_components<BossPart>();

  GameState gs;

  for (auto&& [player, transform] : Zipper(players, transforms)) {
    PlayerState ps;
    ps.playerId = player.player_id;
    ps.posX = transform.position.x;
    ps.posY = transform.position.y;
    ps.hp = static_cast<uint8_t>(player.current);
    ps.weapon = static_cast<uint8_t>(0);
    ps.state = player.isAlive ? 1 : 0;
    gs.players.push_back(ps);
  }

  for (auto&& [idx, enemy, transform] : IndexedZipper(enemies, transforms)) {
    EnemyState es;
    es.enemyId = static_cast<uint16_t>(idx);
    es.enemyType = static_cast<uint8_t>(enemy.type);
    es.posX = transform.position.x;
    es.posY = transform.position.y;
    es.hp = static_cast<uint8_t>(enemy.current);
    es.state = 1;
    es.direction = 0;
    gs.enemies.push_back(es);
  }

  for (auto&& [idx, segment, transform] : IndexedZipper(bosspart, transforms)) {
    EnemyState es;
    es.enemyId = static_cast<uint16_t>(idx);
    es.enemyType = 99;
    es.posX = transform.position.x;
    es.posY = transform.position.y;
    es.hp = 1;
    es.state = 1;
    es.direction = 0;
    gs.enemies.push_back(es);
  }

  for (auto&& [idx, boss, transform] : IndexedZipper(bosses, transforms)) {
    EnemyState bs;
    bs.enemyId = static_cast<uint16_t>(idx);
    bs.enemyType = static_cast<uint8_t>(boss.type);
    bs.posX = transform.position.x;
    bs.posY = transform.position.y;
    bs.hp = static_cast<uint8_t>(boss.current);
    bs.state = 1;
    bs.direction = 0;
    gs.enemies.push_back(bs);
  }

  for (auto&& [idx, proj, transform] : IndexedZipper(projectiles, transforms)) {
    ProjectileState ps;
    ps.projectileId = static_cast<uint16_t>(idx);
    ps.ownerId = proj.ownerId;
    ps.posX = transform.position.x;
    ps.posY = transform.position.y;
    ps.velX = proj.direction.x * proj.speed;
    ps.velY = proj.direction.y * proj.speed;
    ps.damage = static_cast<uint8_t>(proj.damage);
    gs.projectiles.push_back(ps);
  }

  data.Set("game_state", gs);
}


void RtypeScene::HandleEvent(SDL_Event& event) {}


extern "C" {
    Scene* CreateScene() {
        return new RtypeScene();
    }
}
