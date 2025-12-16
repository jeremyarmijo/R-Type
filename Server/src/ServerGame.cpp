#include "include/ServerGame.hpp"

#include <iostream>
#include <queue>
#include <utility>
#include <vector>

#include "Collision/Collision.hpp"
#include "Collision/CollisionController.hpp"
#include "Collision/Items.hpp"
#include "Helpers/EntityHelper.hpp"
#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"

ServerGame::ServerGame()
    : serverRunning(true), gameStarted(false), difficulty(1) {
  SetupDecoder(decode);
  SetupEncoder(encode);
}

void ServerGame::HandleAuth(uint16_t playerId) {
  if (std::find(lobbyPlayers.begin(), lobbyPlayers.end(), playerId) !=
      lobbyPlayers.end())
    return;

  float posY = 200.f + ((playerId - 1) % 4) * 100.f;
  Entity player = createPlayer(registry, {200, posY}, playerId);
  m_players[playerId] = player;
  lobbyPlayers.push_back(playerId);
  std::cout << "Player " << playerId << " joined the lobby ("
            << lobbyPlayers.size() << "/2)" << std::endl;

  if (lobbyPlayers.size() == 2 && !gameStarted) {
    StartGame();
  }
}

void ServerGame::SetupNetworkCallbacks() {
  networkManager.SetMessageCallback([this](const NetworkMessage& msg) {
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

  networkManager.SetConnectionCallback([this](uint16_t client_id) {
    std::cout << "Client " << client_id << " connected!" << std::endl;
  });

  networkManager.SetDisconnectionCallback([this](uint16_t client_id) {
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

bool ServerGame::Initialize(uint16_t tcpPort, uint16_t udpPort, int diff) {
  if (!networkManager.Initialize(tcpPort, udpPort)) {
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
}

bool ServerGame::CheckGameEnded() {
  auto playerArray = registry.get_components<PlayerEntity>();
  if (playerArray.size() <= 0) {
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
      if (protocol == 0) networkManager.BroadcastUDP(msg);
      if (protocol == 2) networkManager.BroadcastTCP(msg);
      continue;
    }
    if (protocol == 0) {
      networkManager.SendTo(msg, true);
      continue;
    }
    if (protocol == 2) {
      networkManager.SendTo(msg, false);
      continue;
    }
  }
}

void ServerGame::Run() {
  while (serverRunning) {
    networkManager.Update();
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

  networkManager.Shutdown();
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

  for (auto&& [player] : Zipper(players)) {
    if (player.invtimer > 0.0f) {
      player.invtimer -= deltaTime;
      if (player.invtimer < 0.0f) {
        player.invtimer = 0.0f;
      }
    }
  }

  player_movement_system(registry);
  physics_movement_system(registry, transforms, rigidbodies, deltaTime, {0, 0});
  enemy_movement_system(registry, transforms, rigidbodies, enemies, players,
                        deltaTime);
  boss_movement_system(registry, transforms, rigidbodies, bosses, deltaTime);
  // Weapon systems
  weapon_cooldown_system(registry, weapons, deltaTime);
  weapon_reload_system(registry, weapons, deltaTime);

  weapon_firing_system(
      registry, weapons, transforms,
      [this](size_t entityId) -> bool {
        auto& playerEntity = registry.get_components<PlayerEntity>();
        if (entityId < playerEntity.size() &&
            playerEntity[entityId].has_value()) {
          auto& state = registry.get_components<InputState>()[entityId];
          // std::cout << "entity = " << entityId << " fire state = " <<
          // state->action1 << std::endl;
          return state->action1;
        }
        return false;
      },
      deltaTime);
  // Projectile systems
  projectile_collision_system(registry, transforms, colliders, projectiles);
  projectile_lifetime_system(registry, projectiles, deltaTime);
  // gamePlay_Collision_system(registry, transforms, colliders, players,
  // enemies,
  //                           bosses, /*items*/
  //                           registry.get_components<Items>(), projectiles);
  enemy_wave_system(registry, enemies, deltaTime, 5, difficulty);
  bounds_check_system(registry, transforms, colliders, rigidbodies);
}

void ServerGame::SendWorldStateToClients() {
  auto& transforms = registry.get_components<Transform>();
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();
  auto& projectiles = registry.get_components<Projectile>();

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

  SendAction(std::make_tuple(Action{ActionType::GAME_STATE, gs}, 0));
}
