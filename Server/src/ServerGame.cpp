#include "include/ServerGame.hpp"

#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "Collision/Collision.hpp"
#include "Collision/CollisionController.hpp"
#include "Collision/Items.hpp"
#include "Helpers/EntityHelper.hpp"
#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "components/BossPart.hpp"
#include "components/Force.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/ForceCtrl.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "dynamicLibLoader/DLLoader.hpp"

ServerGame::ServerGame()
    : serverRunning(true), gameStarted(false), difficulty(1) {
  SetupDecoder(decode);
  SetupEncoder(encode);
  DLLoader<INetworkManager> loader("../src/build/libnetwork_server.so", "EntryPointLib");
  networkManager = std::unique_ptr<INetworkManager>(loader.getInstance());
}

void ServerGame::HandleAuth(uint16_t playerId) {
  if (std::find(lobbyPlayers.begin(), lobbyPlayers.end(), playerId) !=
      lobbyPlayers.end())
    return;

  float posY = 200.f + ((playerId - 1) % 4) * 100.f;
  Entity player = createPlayer(registry, {200, posY}, playerId);
  m_players[playerId] = player;
  Entity forceEntity = createForce(registry, player, {200, posY});
  std::cout << "Force created for player " << playerId << std::endl;

  lobbyPlayers.push_back(playerId);
  std::cout << "Player " << playerId << " joined the lobby ("
            << lobbyPlayers.size() << "/4)" << std::endl;

  if (lobbyPlayers.size() == 4 && !gameStarted) {
    StartGame();
  }
}

void ServerGame::SetupNetworkCallbacks() {
  networkManager->SetMessageCallback([this](const NetworkMessage& msg) {
    Event ev = decode.decode(msg.data);
    uint16_t playerId = msg.client_id;

    {
      std::lock_guard<std::mutex> lock(queueMutex);
      eventQueue.push(std::make_tuple(ev, playerId));
    }

    if (ev.type == EventType::AUTH) {
      std::lock_guard<std::mutex> lock(lobbyMutex);
      HandleAuth(playerId);
    }
  });

  networkManager->SetConnectionCallback([this](uint16_t client_id) {
    std::cout << "Client " << client_id << " connected!" << std::endl;
  });

  networkManager->SetDisconnectionCallback([this](uint16_t client_id) {
    std::cout << "Client " << client_id << " disconnected!" << std::endl;
    std::lock_guard<std::mutex> lock(lobbyMutex);
    auto it = m_players.find(client_id);
    if (it != m_players.end()) {
      Entity playerEntity = it->second;
      registry.kill_entity(playerEntity);
      m_players.erase(it);
    }
    lobbyPlayers.erase(
        std::remove(lobbyPlayers.begin(), lobbyPlayers.end(), client_id),
        lobbyPlayers.end());
  });
}

bool ServerGame::Initialize(uint16_t tcpPort, uint16_t udpPort, int diff,
                            const std::string& host) {
  if (!networkManager->Initialize(tcpPort, udpPort, host)) {
    std::cerr << "Failed to initialize network manager" << std::endl;
    return false;
  }
  difficulty = diff;
  // Physics components
  registry.register_component<Transform>();
  registry.register_component<RigidBody>();
  registry.register_component<BoxCollider>();

  // Player components
  registry.register_component<PlayerEntity>();
  registry.register_component<InputState>();

  // Enemy / Gameplay
  registry.register_component<Enemy>();
  registry.register_component<Boss>();
  registry.register_component<Items>();
  registry.register_component<Collision>();
  registry.register_component<EnemySpawning>();

  // Weapon & Projectile components
  registry.register_component<Weapon>();
  registry.register_component<Projectile>();
  registry.register_component<LevelComponent>();
  registry.register_component<BossPart>();
  registry.register_component<Force>();
  std::cout << "Components registered" << std::endl;
  SetupNetworkCallbacks();
  return true;
}

void ServerGame::InitWorld() {
  // Vector2 playerStartPos1{100.f, 200.f};
  // Vector2 playerStartPos2{100.f, 300.f};  // si 2 joueurs
  // Entity player1 = createPlayer(registry, playerStartPos1, 0);
  // Entity player2 = createPlayer(registry, playerStartPos2, 1);

  // Vector2 enemyPos1{800.f, 150.f};
  // Vector2 enemyPos2{800.f, 250.f};
  // Entity enemy1 = createEnemy(registry, EnemyType::Basic, enemyPos1);
  // Entity enemy2 = createEnemy(registry, EnemyType::Zigzag, enemyPos2);

  // Vector2 bossPos{800.f, 100.f};
  // Entity boss =
  //     createBoss(registry, BossType::BigShip, bossPos, BossPhase::Phase1,
  //     500);

  // std::vector<Vector2> spawnPoints = {{600.f, 100.f}, {600.f, 200.f}};
  // Entity spawner =
  //     createEnemySpawner(registry, spawnPoints, 5.f, 3, EnemyType::Basic);

  // std::cout << "World initialized with players, enemies, and boss."
  //           << std::endl;
}

void ServerGame::StartGame() {
  gameStarted = true;
  networkManager.SetGameStarted(gameStarted);

  levelsData = createLevels();
  currentLevelIndex = 0;
  waitingForNextLevel = false;
  levelTransitionTimer = 0.0f;
  currentLevelEntity =
      createLevelEntity(registry, levelsData[currentLevelIndex]);
  std::cout << "[StartGame] Level " << currentLevelIndex + 1 << " created"
            << std::endl;

  std::cout << "Lobby full! Starting the game!!!!\n";
  gameThread = std::thread(&ServerGame::GameLoop, this);
}

std::optional<std::tuple<Event, uint16_t>> ServerGame::PopEvent() {
  std::lock_guard<std::mutex> lock(queueMutex);
  if (eventQueue.empty()) {
    return std::nullopt;
  }
  auto res = eventQueue.front();
  eventQueue.pop();
  return res;
}

void ServerGame::SendAction(std::tuple<Action, uint16_t> ac) {
  std::lock_guard<std::mutex> lock(queueMutex);
  actionQueue.push(std::move(ac));
}

void ServerGame::EndGame() {
  Action ac;
  GameEnd g;
  g.victory = false;
  ac.type = ActionType::GAME_END;
  ac.data = g;

  SendAction(std::make_tuple(ac, 0));
  gameStarted = false;
  std::cout << "game ended!!" << std::endl;
}

void ServerGame::CheckGameEnded() {
  int validPlayers = 0;
  for (auto& [id, entity] : m_players) {
    bool isValid = registry.is_entity_valid(entity);
    std::cout << "[DEBUG] Player " << id << " entity "
              << static_cast<size_t>(entity) << " valid=" << isValid
              << std::endl;
    if (registry.is_entity_valid(entity)) {
      validPlayers += 1;
    }
  }
  std::cout << "[DEBUG] Valid players: " << validPlayers << std::endl;
  if (validPlayers <= 0) {
    EndGame();
  }
}

void ServerGame::GameLoop() {
  InitWorld();

  const auto frameDuration = std::chrono::milliseconds(16);
  auto lastTime = std::chrono::steady_clock::now();
  int tick = 0;

  Action ac;
  GameStart g;
  g.playerSpawnX = 5;
  g.playerSpawnY = 10;
  g.scrollSpeed = 0;
  ac.type = ActionType::GAME_START;
  ac.data = g;

  SendAction(std::make_tuple(ac, 0));

  while (gameStarted) {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    ReceivePlayerInputs();
    UpdateGameState(deltaTime);
    SendWorldStateToClients();
    CheckGameEnded();

    auto frameTime = std::chrono::steady_clock::now() - currentTime;
    if (frameTime < frameDuration) {
      std::this_thread::sleep_for(frameDuration - frameTime);
    }
    tick++;
  }
}

void ServerGame::SendPacket() {
  std::queue<std::tuple<Action, uint16_t>> localQueue;
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    localQueue.swap(actionQueue);
  }
  while (!localQueue.empty()) {
    auto ac = localQueue.front();
    localQueue.pop();

    Action action = std::get<0>(ac);
    uint16_t clientId = std::get<1>(ac);

    NetworkMessage msg;
    msg.client_id = clientId;

    size_t protocol = UseUdp(action.type);

    msg.data = encode.encode(action, protocol);

    /*std::cout << "Try SEND (clientId = " << clientId
              << ")   (protocol = " << protocol << ")" << std::endl;
    std::cout << "Packet = ";
    for (auto &b : msg.data) {
      std::cout << std::hex << static_cast<int>(b) <<  " ";
    }
    std::cout << std::endl;*/
    if (clientId == 0) {
      if (protocol == 0) networkManager->BroadcastUDP(msg);
      if (protocol == 2) networkManager->BroadcastTCP(msg);
      continue;
    }
    if (protocol == 0) {
      networkManager->SendTo(msg, true);
      continue;
    }
    if (protocol == 2) {
      networkManager->SendTo(msg, false);
      continue;
    }
  }
}

void ServerGame::Run() {
  while (serverRunning) {
    networkManager->Update();
    SendPacket();
    if (!gameStarted && gameThread.joinable()) {
      gameThread.join();
      lobbyPlayers.clear();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void ServerGame::Shutdown() {
  serverRunning = false;
  if (gameThread.joinable()) gameThread.join();

  networkManager->Shutdown();
}

void ServerGame::ReceivePlayerInputs() {
  while (auto evOpt = PopEvent()) {
    auto [event, playerId] = evOpt.value();

    switch (event.type) {
      case EventType::PLAYER_INPUT: {
        const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);

        auto& players = registry.get_components<PlayerEntity>();
        auto& states = registry.get_components<InputState>();

        for (auto&& [player, state] : Zipper(players, states)) {
          if (player.player_id != playerId) continue;

          state.moveLeft = input.left;
          state.moveRight = input.right;
          state.moveDown = input.down;
          state.moveUp = input.up;
          state.action1 = input.fire;
          break;
        }
        break;
      }

      default:
        break;
    }
  }
}

void ServerGame::UpdateGameState(float deltaTime) {
  auto& transforms = registry.get_components<Transform>();
  auto& rigidbodies = registry.get_components<RigidBody>();
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();
  auto& colliders = registry.get_components<BoxCollider>();
  auto& projectiles = registry.get_components<Projectile>();
  auto& weapons = registry.get_components<Weapon>();
  auto& forces = registry.get_components<Force>();
  auto& states = registry.get_components<InputState>();

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
            << " levelTransitionTimer=" << levelTransitionTimer << std::endl;

  auto& levelsDbg = registry.get_components<LevelComponent>();
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

  for (size_t i = 0; i < forces.size(); ++i) {
    if (forces[i].has_value()) {
      auto& f = forces[i].value();
      auto& t = transforms[i].value();
      std::cout << "[DEBUG] Force " << i << " at (" << t.position.x << ", "
                << t.position.y << ")"
                << " state=" << static_cast<int>(f.state) << std::endl;
    }
  }
  if (waitingForNextLevel) {
    levelTransitionTimer += deltaTime;
    std::cout << "[DEBUG] Waiting for next level... " << levelTransitionTimer
              << "/" << TIME_BETWEEN_LEVELS << std::endl;

    if (levelTransitionTimer >= TIME_BETWEEN_LEVELS) {
      levelTransitionTimer = 0.0f;
      waitingForNextLevel = false;
      currentLevelIndex++;

      if (currentLevelIndex >= static_cast<int>(levelsData.size())) {
        Action ac;
        GameEnd g;
        g.victory = true;
        ac.type = ActionType::GAME_END;
        ac.data = g;
        SendAction(std::make_tuple(ac, 0));
        gameStarted = false;
        std::cout << "[Game] VICTORY! All levels completed!" << std::endl;
        return;
      }

      currentLevelEntity =
          createLevelEntity(registry, levelsData[currentLevelIndex]);
      std::cout << "[Game] Level " << (currentLevelIndex + 1)
                << " created (Entity "
                << static_cast<size_t>(currentLevelEntity) << ")" << std::endl;
    }
  } else {
    bool levelFinished =
        update_level_system(registry, deltaTime, currentLevelIndex);
    std::cout << "[DEBUG] update_level_system returned: " << levelFinished
              << std::endl;

    if (levelFinished) {
      std::cout << "[DEBUG] Cleaning up remaining enemies and bosses..."
                << std::endl;

      std::vector<Entity> toKill;

      auto& enemiesCleanup = registry.get_components<Enemy>();
      for (size_t i = 0; i < enemiesCleanup.size(); ++i) {
        if (enemiesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking enemy at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(registry.entity_from_index(i));
        }
      }

      auto& bossesCleanup = registry.get_components<Boss>();
      for (size_t i = 0; i < bossesCleanup.size(); ++i) {
        if (bossesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking boss at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(registry.entity_from_index(i));
        }
      }

      auto& bossPartsCleanup = registry.get_components<BossPart>();
      for (size_t i = 0; i < bossPartsCleanup.size(); ++i) {
        if (bossPartsCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking BossPart at index " << i
                    << " for cleanup" << std::endl;
          toKill.push_back(registry.entity_from_index(i));
        }
      }

      for (Entity e : toKill) {
        std::cout << "[DEBUG] Killing leftover entity: "
                  << static_cast<size_t>(e) << std::endl;
        registry.kill_entity(e);
      }

      std::cout << "[DEBUG] Killing level entity: "
                << static_cast<size_t>(currentLevelEntity) << std::endl;
      registry.kill_entity(currentLevelEntity);

      waitingForNextLevel = true;
      levelTransitionTimer = 0.0f;
      std::cout << "[Game] Level " << (currentLevelIndex + 1)
                << " finished! Waiting " << TIME_BETWEEN_LEVELS << " seconds..."
                << std::endl;
    }
  }
  player_movement_system(registry);
  force_control_system(registry, forces, states, transforms);
  force_movement_system(registry, transforms, rigidbodies, forces, players,
                        deltaTime);
  physics_movement_system(registry, transforms, rigidbodies, deltaTime, {0, 0});
  enemy_movement_system(registry, transforms, rigidbodies, enemies, players,
                        deltaTime);
  boss_movement_system(registry, transforms, rigidbodies, bosses, deltaTime);
  boss_part_system(registry, deltaTime);
  weapon_cooldown_system(registry, weapons, deltaTime);
  weapon_reload_system(registry, weapons, deltaTime);
  weapon_firing_system(
      registry, weapons, transforms,
      [this](size_t entityId) -> bool {
        auto& playerEntity = registry.get_components<PlayerEntity>();
        if (entityId < playerEntity.size() &&
            playerEntity[entityId].has_value()) {
          auto& state = registry.get_components<InputState>()[entityId];
          return state->action1;
        }
        return false;
      },
      deltaTime);

  projectile_collision_system(registry, transforms, colliders, projectiles);
  projectile_lifetime_system(registry, projectiles, deltaTime);
  gamePlay_Collision_system(registry, transforms, colliders, players, enemies,
                            bosses);
  bounds_check_system(registry, transforms, colliders, rigidbodies);
  force_collision_system(registry, transforms, colliders, forces, enemies,
                         bosses, registry.get_components<BossPart>(),
                         projectiles);
}

void ServerGame::SendWorldStateToClients() {
  auto& transforms = registry.get_components<Transform>();
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();
  auto& projectiles = registry.get_components<Projectile>();
  auto& bosspart = registry.get_components<BossPart>();
  auto& forcesArr = registry.get_components<Force>();

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

  /*for (auto&& [idx, force, transform] : IndexedZipper(forcesArr, transforms))
{ if (!force.isActive) continue;

    ForceState fs;  // Tu dois créer cette structure
    fs.forceId = static_cast<uint16_t>(idx);
    fs.ownerId = static_cast<uint16_t>(force.ownerPlayer);
    fs.posX = transform.position.x;
    fs.posY = transform.position.y;
    fs.state = static_cast<uint8_t>(force.state);
    gs.forces.push_back(fs);
  }

  struct ForceState {
  uint16_t forceId;
  uint16_t ownerId;
  float posX;
  float posY;
  uint8_t state;  // 0=AttachedFront, 1=AttachedBack, 2=Detached
};*/
  SendAction(std::make_tuple(Action{ActionType::GAME_STATE, gs}, 0));
}
