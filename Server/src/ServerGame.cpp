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
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "dynamicLibLoader/DLLoader.hpp"

ServerGame::ServerGame() : serverRunning(true) {
  SetupDecoder(decode);
  SetupEncoder(encode);
  DLLoader<INetworkManager> loader("../src/build/libnetwork_server.so", "EntryPointLib");
  networkManager = std::unique_ptr<INetworkManager>(loader.getInstance());
}

void ServerGame::CreateLobby(uint16_t playerId, std::string mdp,
                             uint8_t difficulty) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  auto l = std::make_unique<lobby_list>();
  l->lobby_id = nextLobbyId++;
  l->mdp = mdp;
  l->difficulty = difficulty;
  l->hasPassword = !mdp.empty();
  l->nb_player = 1;
  l->players_list.push_back(std::make_tuple(playerId, false));

  std::cout << "[Lobby] Player " << playerId << " created lobby " << l->lobby_id
            << std::endl;

  Action ac;
  LobbyJoinResponse resp;
  resp.success = true;
  resp.lobbyId = l->lobby_id;
  resp.playerId = playerId;

  LobbyPlayer p;
  p.playerId = playerId;
  p.ready = false;
  p.username = "Player" + std::to_string(playerId);
  resp.players.push_back(p);

  ac.type = ActionType::LOBBY_JOIN_RESPONSE;
  ac.data = resp;
  SendAction(std::make_tuple(ac, playerId, nullptr));

  lobbys.push_back(std::move(l));
}

void ServerGame::SendLobbyUpdate(lobby_list& lobby) {
  Action ac;
  LobbyUpdate update;

  for (auto& [pId, ready] : lobby.players_list) {
    LobbyPlayer p;
    p.playerId = pId;
    p.ready = ready;
    p.username = "Player" + std::to_string(pId);
    update.playerInfo.push_back(p);
  }

  ac.type = ActionType::LOBBY_UPDATE;
  ac.data = update;

  SendAction(std::make_tuple(ac, 0, &lobby));
}

void ServerGame::JoinLobby(uint16_t playerId, uint16_t lobbyId,
                           std::string mdp) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  Action ac;
  LobbyJoinResponse resp;

  lobby_list* targetLobby = nullptr;
  for (auto& lobby : lobbys) {
    if (lobby->lobby_id == lobbyId) {
      targetLobby = lobby.get();
      break;
    }
  }

  if (!targetLobby) {
    resp.success = false;
    resp.errorCode = 0x2010;
    resp.errorMessage = "Lobby not found";
  } else if (targetLobby->gameRuning) {
    resp.success = false;
    resp.errorCode = 0x2014;
    resp.errorMessage = "Game already started";
  } else if (targetLobby->nb_player >= targetLobby->max_players) {
    resp.success = false;
    resp.errorCode = 0x2012;
    resp.errorMessage = "Lobby is full";
  } else if (targetLobby->hasPassword && targetLobby->mdp != mdp) {
    resp.success = false;
    resp.errorCode = 0x2011;
    resp.errorMessage = "Invalid password";
  } else {
    targetLobby->players_list.push_back(std::make_tuple(playerId, false));
    targetLobby->nb_player++;

    resp.success = true;
    resp.lobbyId = lobbyId;
    resp.playerId = playerId;

    for (auto& [pId, ready] : targetLobby->players_list) {
      LobbyPlayer p;
      p.playerId = pId;
      p.ready = ready;
      p.username = "Player" + std::to_string(pId);
      resp.players.push_back(p);
    }

    std::cout << "[Lobby] Player " << playerId << " joined lobby " << lobbyId
              << std::endl;
    SendLobbyUpdate(*targetLobby);
  }

  ac.type = ActionType::LOBBY_JOIN_RESPONSE;
  ac.data = resp;
  SendAction(std::make_tuple(ac, playerId, nullptr));
}

void ServerGame::RemovePlayerFromLobby(uint16_t playerId) {
  for (auto it = lobbys.begin(); it != lobbys.end();) {
    bool found = false;

    for (auto pIt = (*it)->players_list.begin();
         pIt != (*it)->players_list.end(); ++pIt) {
      if (std::get<0>(*pIt) == playerId) {
        (*it)->players_list.erase(pIt);
        (*it)->nb_player--;
        found = true;
        break;
      }
    }

    if (found) {
      if ((*it)->nb_player == 0) {
        std::cout << "[Lobby] Lobby " << (*it)->lobby_id << " deleted (empty)"
                  << std::endl;
        (*it)->gameRuning = false;
        if ((*it)->gameThread.joinable()) {
          (*it)->gameThread.join();
        }
        it = lobbys.erase(it);
      } else {
        SendLobbyUpdate(**it);
        ++it;
      }
      break;
    } else {
      ++it;
    }
  }
}

void ServerGame::HandlePayerReady(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  for (auto& lobby : lobbys) {
    uint16_t nbPlayerReady = 0;
    bool found = false;

    for (auto& player : lobby->players_list) {
      if (std::get<0>(player) == playerId) {
        std::get<1>(player) = !std::get<1>(player);
        found = true;
      }
      if (std::get<1>(player) == true) {
        nbPlayerReady++;
      }
    }

    if (found) {
      SendLobbyUpdate(*lobby);

      if (nbPlayerReady == lobby->nb_player && !lobby->gameRuning) {
        lobby->players_ready = true;

        Action ac;
        LobbyStart start;
        start.countdown = 3;
        ac.type = ActionType::LOBBY_START;
        ac.data = start;

        for (auto& [pId, ready] : lobby->players_list) {
          SendAction(std::make_tuple(ac, pId, nullptr));
        }
        lobby->gameRuning = true;
        StartGame(*lobby);
      }
      break;
    }
  }
}

void ServerGame::HandleLobbyCreate(uint16_t playerId, Event& ev) {
  const auto* create = std::get_if<LOBBY_CREATE>(&ev.data);
  if (!create) return;

  CreateLobby(playerId, create->password, create->difficulty);
}

void ServerGame::HandleLobbyJoinRequest(uint16_t playerId, Event& ev) {
  const auto* join = std::get_if<LOBBY_JOIN_REQUEST>(&ev.data);
  if (!join) return;

  JoinLobby(playerId, join->lobbyId, join->password);
}

void ServerGame::HandleLobbyListRequest(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  Action ac;
  LobbyListResponse resp;

  for (const auto& lobby : lobbys) {
    if (!lobby->gameRuning) {
      LobbyInfo info;
      info.lobbyId = lobby->lobby_id;
      info.playerCount = lobby->nb_player;
      info.maxPlayers = lobby->max_players;
      info.difficulty = lobby->difficulty;
      info.isStarted = lobby->gameRuning;
      info.hasPassword = lobby->hasPassword;
      resp.lobbies.push_back(info);
    }
  }

  ac.type = ActionType::LOBBY_LIST_RESPONSE;
  ac.data = resp;
  SendAction(std::make_tuple(ac, playerId, nullptr));
}

void ServerGame::HandleLobbyLeave(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);
  RemovePlayerFromLobby(playerId);
}

lobby_list* ServerGame::FindPlayerLobby(uint16_t playerId) {
  for (auto& lobby : lobbys) {
    for (auto& [pId, ready] : lobby->players_list) {
      if (pId == playerId) {
        return lobby.get();
      }
    }
  }
  return nullptr;
}

void ServerGame::SetupNetworkCallbacks() {
  networkManager->SetMessageCallback([this](const NetworkMessage& msg) {
    Event ev = decode.decode(msg.data);
    uint16_t playerId = msg.client_id;

    {
      std::lock_guard<std::mutex> lock(queueMutex);
      eventQueue.push(std::make_tuple(ev, playerId));
    }

    if (ev.type == EventType::LOBBY_CREATE) {
      HandleLobbyCreate(playerId, ev);
    } else if (ev.type == EventType::LOBBY_JOIN_REQUEST) {
      HandleLobbyJoinRequest(playerId, ev);
    } else if (ev.type == EventType::LOBBY_LIST_REQUEST) {
      HandleLobbyListRequest(playerId);
    } else if (ev.type == EventType::LOBBY_LEAVE) {
      HandleLobbyLeave(playerId);
    } else if (ev.type == EventType::PLAYER_READY) {
      HandlePayerReady(playerId);
    }
  });

  networkManager->SetConnectionCallback([this](uint16_t client_id) {
    std::cout << "Client " << client_id << " connected!" << std::endl;
  });

  networkManager->SetDisconnectionCallback([this](uint16_t client_id) {
    std::cout << "Client " << client_id << " disconnected!" << std::endl;

    RemovePlayerFromLobby(client_id);

    std::lock_guard<std::mutex> lock(lobbyMutex);
    for (auto& lobby : lobbys) {
      auto it = lobby->m_players.find(client_id);
      if (it != lobby->m_players.end()) {
        Entity playerEntity = it->second;
        lobby->registry.kill_entity(playerEntity);
        lobby->m_players.erase(it);
      }
    }
  });
}
bool ServerGame::Initialize(uint16_t tcpPort, uint16_t udpPort, int diff,
                            const std::string& host) {
  if (!networkManager->Initialize(tcpPort, udpPort, host)) {
    std::cerr << "Failed to initialize network manager" << std::endl;
    return false;
  }
  std::cout << "Components registered" << std::endl;
  SetupNetworkCallbacks();
  return true;
}

void ServerGame::InitWorld(lobby_list& lobby) {
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

void ServerGame::StartGame(lobby_list& lobby_list) {
  // Physics components
  lobby_list.registry.register_component<Transform>();
  lobby_list.registry.register_component<RigidBody>();
  lobby_list.registry.register_component<BoxCollider>();

  // Player components
  lobby_list.registry.register_component<PlayerEntity>();
  lobby_list.registry.register_component<InputState>();

  // Enemy / Gameplay
  lobby_list.registry.register_component<Enemy>();
  lobby_list.registry.register_component<Boss>();
  lobby_list.registry.register_component<Items>();
  lobby_list.registry.register_component<Collision>();
  lobby_list.registry.register_component<EnemySpawning>();

  // Weapon & Projectile components
  lobby_list.registry.register_component<Weapon>();
  lobby_list.registry.register_component<Projectile>();
  lobby_list.registry.register_component<LevelComponent>();
  lobby_list.registry.register_component<BossPart>();

  for (auto& [playerId, ready] : lobby_list.players_list) {
    float posY = 200.f + (lobby_list.m_players.size() % 4) * 100.f;
    Entity player = createPlayer(lobby_list.registry, {200, posY}, playerId);
    lobby_list.m_players[playerId] = player;
  }

  lobby_list.levelsData = createLevels();
  lobby_list.currentLevelIndex = 0;
  lobby_list.waitingForNextLevel = false;
  lobby_list.levelTransitionTimer = 0.0f;
  lobby_list.currentLevelEntity = createLevelEntity(
      lobby_list.registry, lobby_list.levelsData[lobby_list.currentLevelIndex]);
  std::cout << "[StartGame] Level " << lobby_list.currentLevelIndex + 1
            << " created" << std::endl;

  std::cout << "Lobby full! Starting the game!!!!\n";
  lobby_list.gameThread =
      std::thread(&ServerGame::GameLoop, this, std::ref(lobby_list));
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

void ServerGame::SendAction(std::tuple<Action, uint16_t, lobby_list*> ac) {
  std::lock_guard<std::mutex> lock(queueMutex);
  actionQueue.push(std::move(ac));
}

void ServerGame::EndGame(lobby_list& lobby) {
  Action ac;
  GameEnd g;
  g.victory = false;
  ac.type = ActionType::GAME_END;
  ac.data = g;

  SendAction(std::make_tuple(ac, 0, &lobby));
  lobby.gameRuning = false;
  std::cout << "game ended!!" << std::endl;
}

void ServerGame::CheckGameEnded(lobby_list& lobby) {
  int validPlayers = 0;
  for (auto& [id, entity] : lobby.m_players) {
    if (lobby.registry.is_entity_valid(entity)) {
      validPlayers += 1;
    }
  }
  if (validPlayers <= 0) {
    EndGame(lobby);
  }
}

void ServerGame::GameLoop(lobby_list& lobby) {
  InitWorld(lobby);

  std::this_thread::sleep_for(std::chrono::seconds(3));

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

  SendAction(std::make_tuple(ac, 0, &lobby));

  while (lobby.gameRuning) {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    ReceivePlayerInputs(lobby);
    UpdateGameState(lobby, deltaTime);
    SendWorldStateToClients(lobby);
    CheckGameEnded(lobby);

    auto frameTime = std::chrono::steady_clock::now() - currentTime;
    if (frameTime < frameDuration) {
      std::this_thread::sleep_for(frameDuration - frameTime);
    }
    tick++;
  }
}

void ServerGame::SendPacket() {
  std::queue<std::tuple<Action, uint16_t, lobby_list*>> localQueue;
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    localQueue.swap(actionQueue);
  }
  while (!localQueue.empty()) {
    auto ac = localQueue.front();
    localQueue.pop();

    Action action = std::get<0>(ac);
    uint16_t clientId = std::get<1>(ac);
    lobby_list* lobby = std::get<2>(ac);

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
    if (clientId == 0 && lobby != nullptr) {
      if (protocol == 0)
        networkManager->BroadcastLobbyUDP(msg, lobby->players_list);
      if (protocol == 2)
        networkManager->BroadcastLobbyTCP(msg, lobby->players_list);
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
    for (auto& lobby : lobbys) {
      if (!lobby->gameRuning && lobby->gameThread.joinable()) {
        lobby->gameThread.join();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void ServerGame::Shutdown() {
  serverRunning = false;
  for (auto& lobby : lobbys) {
    if (lobby->gameThread.joinable()) {
      lobby->gameThread.join();
    }
  }
  networkManager->Shutdown();
}

void ServerGame::ReceivePlayerInputs(lobby_list& lobby) {
  while (auto evOpt = PopEvent()) {
    auto [event, playerId] = evOpt.value();

    bool playerInLobby = false;
    for (auto& [pId, ready] : lobby.players_list) {
      if (pId == playerId) {
        playerInLobby = true;
        break;
      }
    }
    if (!playerInLobby) continue;

    switch (event.type) {
      case EventType::PLAYER_INPUT: {
        const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);

        auto& players = lobby.registry.get_components<PlayerEntity>();
        auto& states = lobby.registry.get_components<InputState>();

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

void ServerGame::UpdateGameState(lobby_list& lobby, float deltaTime) {
  auto& transforms = lobby.registry.get_components<Transform>();
  auto& rigidbodies = lobby.registry.get_components<RigidBody>();
  auto& players = lobby.registry.get_components<PlayerEntity>();
  auto& enemies = lobby.registry.get_components<Enemy>();
  auto& bosses = lobby.registry.get_components<Boss>();
  auto& colliders = lobby.registry.get_components<BoxCollider>();
  auto& projectiles = lobby.registry.get_components<Projectile>();
  auto& weapons = lobby.registry.get_components<Weapon>();

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

  std::cout << "[DEBUG] currentLevelIndex=" << lobby.currentLevelIndex
            << " waitingForNextLevel=" << lobby.waitingForNextLevel
            << " levelTransitionTimer=" << lobby.levelTransitionTimer
            << std::endl;

  auto& levelsDbg = lobby.registry.get_components<LevelComponent>();
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

  if (lobby.waitingForNextLevel) {
    lobby.levelTransitionTimer += deltaTime;
    std::cout << "[DEBUG] Waiting for next level... "
              << lobby.levelTransitionTimer << "/" << TIME_BETWEEN_LEVELS
              << std::endl;

    if (lobby.levelTransitionTimer >= TIME_BETWEEN_LEVELS) {
      lobby.levelTransitionTimer = 0.0f;
      lobby.waitingForNextLevel = false;
      lobby.currentLevelIndex++;

      if (lobby.currentLevelIndex >=
          static_cast<int>(lobby.levelsData.size())) {
        Action ac;
        GameEnd g;
        g.victory = true;
        ac.type = ActionType::GAME_END;
        ac.data = g;
        SendAction(std::make_tuple(ac, 0, &lobby));
        lobby.gameRuning = false;
        std::cout << "[Game] VICTORY! All levels completed!" << std::endl;
        return;
      }

      lobby.currentLevelEntity = createLevelEntity(
          lobby.registry, lobby.levelsData[lobby.currentLevelIndex]);
      std::cout << "[Game] Level " << (lobby.currentLevelIndex + 1)
                << " created (Entity "
                << static_cast<size_t>(lobby.currentLevelEntity) << ")"
                << std::endl;
    }
  } else {
    bool levelFinished =
        update_level_system(lobby.registry, deltaTime, lobby.currentLevelIndex);
    std::cout << "[DEBUG] update_level_system returned: " << levelFinished
              << std::endl;

    if (levelFinished) {
      std::cout << "[DEBUG] Cleaning up remaining enemies and bosses..."
                << std::endl;

      std::vector<Entity> toKill;

      auto& enemiesCleanup = lobby.registry.get_components<Enemy>();
      for (size_t i = 0; i < enemiesCleanup.size(); ++i) {
        if (enemiesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking enemy at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(lobby.registry.entity_from_index(i));
        }
      }

      auto& bossesCleanup = lobby.registry.get_components<Boss>();
      for (size_t i = 0; i < bossesCleanup.size(); ++i) {
        if (bossesCleanup[i].has_value()) {
          std::cout << "[DEBUG] Marking boss at index " << i << " for cleanup"
                    << std::endl;
          toKill.push_back(lobby.registry.entity_from_index(i));
        }
      }
      for (Entity e : toKill) {
        std::cout << "[DEBUG] Killing leftover entity: "
                  << static_cast<size_t>(e) << std::endl;
        lobby.registry.kill_entity(e);
      }

      std::cout << "[DEBUG] Killing level entity: "
                << static_cast<size_t>(lobby.currentLevelEntity) << std::endl;
      lobby.registry.kill_entity(lobby.currentLevelEntity);

      lobby.waitingForNextLevel = true;
      lobby.levelTransitionTimer = 0.0f;
      std::cout << "[Game] Level " << (lobby.currentLevelIndex + 1)
                << " finished! Waiting " << TIME_BETWEEN_LEVELS << " seconds..."
                << std::endl;
    }
  }
  player_movement_system(lobby.registry);
  physics_movement_system(lobby.registry, transforms, rigidbodies, deltaTime,
                          {0, 0});
  enemy_movement_system(lobby.registry, transforms, rigidbodies, enemies,
                        players, deltaTime);
  boss_movement_system(lobby.registry, transforms, rigidbodies, bosses,
                       deltaTime);
  boss_part_system(lobby.registry, deltaTime);
  weapon_cooldown_system(lobby.registry, weapons, deltaTime);
  weapon_reload_system(lobby.registry, weapons, deltaTime);
  weapon_firing_system(
      lobby.registry, weapons, transforms,
      [&lobby](size_t entityId) -> bool {
        auto& playerEntity = lobby.registry.get_components<PlayerEntity>();
        if (entityId < playerEntity.size() &&
            playerEntity[entityId].has_value()) {
          auto& state = lobby.registry.get_components<InputState>()[entityId];
          return state->action1;
        }
        return false;
      },
      deltaTime);

  projectile_collision_system(lobby.registry, transforms, colliders,
                              projectiles);
  projectile_lifetime_system(lobby.registry, projectiles, deltaTime);
  gamePlay_Collision_system(lobby.registry, transforms, colliders, players,
                            enemies, bosses);
  bounds_check_system(lobby.registry, transforms, colliders, rigidbodies);
}

void ServerGame::SendWorldStateToClients(lobby_list& lobby) {
  auto& transforms = lobby.registry.get_components<Transform>();
  auto& players = lobby.registry.get_components<PlayerEntity>();
  auto& enemies = lobby.registry.get_components<Enemy>();
  auto& bosses = lobby.registry.get_components<Boss>();
  auto& projectiles = lobby.registry.get_components<Projectile>();
  auto& bosspart = lobby.registry.get_components<BossPart>();

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

  SendAction(std::make_tuple(Action{ActionType::GAME_STATE, gs}, 0, &lobby));
}
