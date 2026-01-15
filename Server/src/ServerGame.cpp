#include "include/ServerGame.hpp"

#include <iostream>
#include <memory>
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
#include "dynamicLibLoader/DLLoader.hpp"
#include "ecs/Registry.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"

ServerGame::ServerGame() : serverRunning(true) {
  SetupDecoder(decode);
  SetupEncoder(encode);
  networkManager = std::unique_ptr<INetworkManager>(loader.getInstance());
}

void ServerGame::CreateLobby(uint16_t playerId, std::string lobbyName,
                             std::string playerName, std::string mdp,
                             uint8_t difficulty, uint8_t Maxplayer) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  auto l = std::make_unique<lobby_list>();
  l->lobby_id = nextLobbyId++;
  l->host_id = playerId;
  l->name = lobbyName;
  l->max_players = Maxplayer;
  l->mdp = mdp;
  l->difficulty = difficulty;
  l->hasPassword = !mdp.empty();
  l->nb_player = 1;
  l->players_list.push_back(std::make_tuple(playerId, false, playerName));

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

  lobby_list* lobbyPtr = l.get();

  lobbys.push_back(std::move(l));
  SendLobbyUpdate(*lobbyPtr);
}

void ServerGame::SendLobbyUpdate(lobby_list& lobby) {
  Action ac;
  LobbyUpdate update;

  for (auto& [pId, ready, pName] : lobby.players_list) {
    LobbyPlayer p;
    p.playerId = pId;
    p.ready = ready;
    p.username = pName;

    update.difficulty = lobby.difficulty;
    update.hostId = lobby.host_id;
    update.maxPlayers = lobby.max_players;
    update.asStarted = lobby.gameRuning;
    update.name = lobby.name;
    update.playerInfo.push_back(p);
  }

  ac.type = ActionType::LOBBY_UPDATE;
  ac.data = update;

  SendAction(std::make_tuple(ac, 0, &lobby));
}

void ServerGame::JoinLobby(uint16_t playerId, std::string playerName,
                           uint16_t lobbyId, std::string mdp) {
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

    ac.type = ActionType::LOBBY_JOIN_RESPONSE;
    ac.data = resp;
    SendAction(std::make_tuple(ac, playerId, nullptr));

  } else if (targetLobby->nb_player >= targetLobby->max_players) {
    resp.success = false;
    resp.errorCode = 0x2012;
    resp.errorMessage = "Lobby is full";

    ac.type = ActionType::LOBBY_JOIN_RESPONSE;
    ac.data = resp;
    SendAction(std::make_tuple(ac, playerId, nullptr));

  } else if (targetLobby->hasPassword && targetLobby->mdp != mdp) {
    resp.success = false;
    resp.errorCode = 0x2011;
    resp.errorMessage = "Invalid password";

    ac.type = ActionType::LOBBY_JOIN_RESPONSE;
    ac.data = resp;
    SendAction(std::make_tuple(ac, playerId, nullptr));
  } else {
    if (targetLobby->gameRuning) {
      targetLobby->spectate.push_back(
          std::make_tuple(playerId, false, playerName));
    } else {
      targetLobby->players_list.push_back(
          std::make_tuple(playerId, false, playerName));
    }
    targetLobby->nb_player++;

    resp.success = true;
    resp.lobbyId = lobbyId;
    resp.playerId = playerId;

    for (auto& [pId, ready, pName] : targetLobby->players_list) {
      LobbyPlayer p;
      p.playerId = pId;
      p.ready = ready;
      p.username = pName;
      resp.players.push_back(p);
    }

    ac.type = ActionType::LOBBY_JOIN_RESPONSE;
    ac.data = resp;
    SendAction(std::make_tuple(ac, playerId, nullptr));

    std::cout << "[Lobby] Player " << playerId << " joined lobby " << lobbyId
              << std::endl;
    SendLobbyUpdate(*targetLobby);
  }
}

void ServerGame::RemovePlayerFromLobby(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);
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

void ServerGame::ResetLobbyReadyStatus(lobby_list& lobby) {
  for (auto& player : lobby.players_list) {
    std::get<1>(player) = false;
  }

  lobby.players_ready = false;

  SendLobbyUpdate(lobby);
}

void ServerGame::HandlePayerReady(uint16_t playerId, bool isReady) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  for (auto& lobby : lobbys) {
    uint16_t nbPlayerReady = 0;
    bool found = false;

    for (auto& player : lobby->players_list) {
      if (std::get<0>(player) == playerId) {
        std::get<1>(player) = isReady;
        found = true;
      }
      if (std::get<1>(player) == true) {
        nbPlayerReady++;
      }
    }

    for (auto& player : lobby->spectate) {
      if (std::get<0>(player) == playerId) {
        std::get<1>(player) = isReady;
        found = true;
      }
    }

    if (found) {
      SendLobbyUpdate(*lobby);
      if (lobby->gameRuning && isReady) {
        Action startAc;
        GameStart gs;
        gs.playerSpawnX = 0;
        gs.playerSpawnY = 0;
        gs.scrollSpeed = 0;
        startAc.type = ActionType::GAME_START;
        startAc.data = gs;
        SendAction(std::make_tuple(startAc, playerId, nullptr));
        return;
      }
      if (nbPlayerReady == lobby->nb_player && !lobby->gameRuning) {
        lobby->players_ready = true;

        Action ac;
        LobbyStart start;
        start.countdown = 3;
        ac.type = ActionType::LOBBY_START;
        ac.data = start;

        for (auto& [pId, ready, _] : lobby->players_list) {
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

  CreateLobby(playerId, create->lobbyName, create->playerName, create->password,
              create->difficulty, create->Maxplayer);
}

void ServerGame::HandleLobbyJoinRequest(uint16_t playerId, Event& ev) {
  const auto* join = std::get_if<LOBBY_JOIN_REQUEST>(&ev.data);
  if (!join) return;

  JoinLobby(playerId, join->name, join->lobbyId, join->password);
}

void ServerGame::HandleLobbyListRequest(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);

  Action ac;
  LobbyListResponse resp;

  for (const auto& lobby : lobbys) {
    LobbyInfo info;
    info.lobbyId = lobby->lobby_id;
    info.name = lobby->name;
    info.playerCount = lobby->nb_player;
    info.maxPlayers = lobby->max_players;
    info.difficulty = lobby->difficulty;
    info.isStarted = lobby->gameRuning;
    info.hasPassword = lobby->hasPassword;
    resp.lobbies.push_back(info);
  }

  ac.type = ActionType::LOBBY_LIST_RESPONSE;
  ac.data = resp;
  SendAction(std::make_tuple(ac, playerId, nullptr));
}

void ServerGame::HandleLobbyLeave(uint16_t playerId) {
  RemovePlayerFromLobby(playerId);
}

void ServerGame::HandleLobbyMessage(uint16_t playerId, Event& ev) {
  const auto* msgData = std::get_if<MESSAGE>(&ev.data);
  if (!msgData) return;

  lobby_list* lobby = FindPlayerLobby(playerId);
  if (!lobby) return;

  Action ac;
  Message response;
  response.lobbyId = lobby->lobby_id;
  response.playerName = msgData->playerName;

  if (msgData->message.empty() || msgData->message == "") {
    response.message = " ";
  } else {
    response.message = msgData->message;
  }

  ac.type = ActionType::MESSAGE;
  ac.data = response;

  std::cout << "[Chat] [" << lobby->name << "] " << msgData->playerName << ": "
            << msgData->message << std::endl;
  SendAction(std::make_tuple(ac, 0, lobby));
}

lobby_list* ServerGame::FindPlayerLobby(uint16_t playerId) {
  std::lock_guard<std::mutex> lock(lobbyMutex);
  for (auto& lobby : lobbys) {
    for (auto& [pId, ready, _] : lobby->players_list) {
      if (pId == playerId) {
        return lobby.get();
      }
    }
  }
  return nullptr;
}

void ServerGame::HandleLoginResponse(uint16_t playerId, Event& ev) {
  const auto* login = std::get_if<LOGIN_REQUEST>(&ev.data);
  if (!login) return;
  LoginResponse lr;
  lr.success = 1;
  lr.playerId = playerId;
  lr.udpPort = 4243;

  Action res;
  res.type = ActionType::LOGIN_RESPONSE;
  res.data = lr;

  SendAction(std::make_tuple(res, playerId, nullptr));
}

void ServerGame::SetupNetworkCallbacks() {
  networkManager->SetMessageCallback([this](const NetworkMessage& msg) {
    Event ev = decode.decode(msg.data);
    uint16_t playerId = msg.client_id;

    lobby_list* targetLobby = FindPlayerLobby(playerId);

    if (targetLobby && targetLobby->gameRuning &&
        ev.type == EventType::PLAYER_INPUT) {
      std::lock_guard<std::mutex> lock(targetLobby->lobbyQueueMutex);
      targetLobby->lobbyEventQueue.push({ev, playerId});
      return;
    }

    {
      std::lock_guard<std::mutex> lock(queueMutex);
      eventQueue.push(std::make_tuple(ev, playerId));
    }

    switch (ev.type) {
      case EventType::LOGIN_REQUEST: {
        std::cout << "[ServerGame] Login request from: " << playerId
                  << std::endl;
        HandleLoginResponse(playerId, ev);
        break;
      }
      case EventType::LOBBY_CREATE:
        std::cout << "[Network] LOBBY_CREATE from " << playerId << std::endl;
        HandleLobbyCreate(playerId, ev);
        break;

      case EventType::LOBBY_JOIN_REQUEST:
        std::cout << "[Network] LOBBY_JOIN_REQUEST from " << playerId
                  << std::endl;
        HandleLobbyJoinRequest(playerId, ev);
        break;

      case EventType::LOBBY_LIST_REQUEST:
        HandleLobbyListRequest(playerId);
        break;

      case EventType::PLAYER_READY:
        std::cout << "[Network] PLAYER_READY from " << playerId << std::endl;
        HandlePayerReady(playerId, std::get<PLAYER_READY>(ev.data).ready);
        break;

      case EventType::LOBBY_LEAVE:
        std::cout << "[Network] LOBBY_LEAVE from " << playerId << std::endl;
        HandleLobbyLeave(playerId);
        break;

      case EventType::LOBBY_KICK: {
        auto& d = std::get<LOBBY_KICK>(ev.data);
        std::cout << "[Network] LOBBY_LEAVE from " << playerId << std::endl;
        HandleLobbyLeave(d.playerId);
        break;
      }
      case EventType::MESSAGE:
        HandleLobbyMessage(playerId, ev);
        break;

      case EventType::ERROR_TYPE: {
        auto& d = std::get<ERROR_EVNT>(ev.data);
        std::cerr << "[Network Error] Client " << playerId << ": " << d.message
                  << std::endl;
        break;
      }

      default:
        break;
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

  for (auto& [playerId, ready, _] : lobby_list.players_list) {
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
  if (!lobby.gameRuning) return;

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

  for (int i = 0; i < 100 && lobby.gameRuning; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  const auto frameDuration = std::chrono::milliseconds(16);
  auto lastTime = std::chrono::steady_clock::now();

  if (lobby.gameRuning) {
    Action ac;
    GameStart g;
    g.playerSpawnX = 5;
    g.playerSpawnY = 10;
    g.scrollSpeed = 0;
    ac.type = ActionType::GAME_START;
    ac.data = g;
    SendAction(std::make_tuple(ac, 0, &lobby));
  }

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
    if (frameTime < frameDuration && lobby.gameRuning) {
      std::this_thread::sleep_for(frameDuration - frameTime);
    }
  }
  std::cout << "[Thread] GameLoop stopped for lobby " << lobby.lobby_id
            << std::endl;
}

void ServerGame::SendPacket() {
  std::queue<std::tuple<Action, uint16_t, lobby_list*>> localQueue;
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    localQueue.swap(actionQueue);
  }
  while (!localQueue.empty()) {
    auto& [action, clientId, lobby] = localQueue.front();

    if (clientId == 0 && lobby != nullptr) {
      size_t protocol = UseUdp(action.type);

      if (protocol == 0 || protocol == 1) {
        networkManager->BroadcastLobbyUDP(action, lobby->players_list);
        networkManager->BroadcastLobbyUDP(action, lobby->spectate);
      } else {
        networkManager->BroadcastLobbyTCP(action, lobby->players_list);
        networkManager->BroadcastLobbyTCP(action, lobby->spectate);
      }
    } else {
      NetworkMessage msg;
      msg.client_id = clientId;
      networkManager->SendTo(msg, action);
    }
    localQueue.pop();
  }
}

void ServerGame::ClearLobbyForRematch(lobby_list& lobby) {
  lobby.registry = Registry{};
  lobby.m_players.clear();
  lobby.currentLevelIndex = 0;
  lobby.waitingForNextLevel = false;
  lobby.levelTransitionTimer = 0.0f;

  for (auto& player : lobby.players_list) {
    std::get<1>(player) = false;
  }
  for (auto& spectator : lobby.spectate) {
    if (lobby.players_list.size() < lobby.max_players) {
      std::get<1>(spectator) = false;
      lobby.players_list.push_back(spectator);
    }
  }
  lobby.spectate.clear();
  lobby.players_ready = false;

  SendLobbyUpdate(lobby);
}

void ServerGame::Run() {
  std::cout << "before main loop" << std::endl;

  while (serverRunning) {
    networkManager->Update();
    SendPacket();

    {
      std::lock_guard<std::mutex> lock(lobbyMutex);
      for (auto it = lobbys.begin(); it != lobbys.end();) {
        if (!(*it)->gameRuning && (*it)->gameThread.joinable()) {
          (*it)->gameThread.join();
          ClearLobbyForRematch(**it);
          std::cout << "[Lobby] " << (*it)->lobby_id << " reset done."
                    << std::endl;
        }
        ++it;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  std::cout << "server " << std::endl;
}

void ServerGame::Shutdown() {
  serverRunning = false;
  {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    for (auto& lobby : lobbys) {
      lobby->gameRuning = false;
      if (lobby->gameThread.joinable()) {
        lobby->gameThread.join();
      }
    }
  }
  networkManager->Shutdown();
}

std::optional<std::pair<Event, uint16_t>> ServerGame::PopEventLobby(
    lobby_list& lobby) {
  std::lock_guard<std::mutex> lock(lobby.lobbyQueueMutex);

  if (lobby.lobbyEventQueue.empty()) {
    return std::nullopt;
  }

  auto res = lobby.lobbyEventQueue.front();
  lobby.lobbyEventQueue.pop();
  return res;
}

void ServerGame::ReceivePlayerInputs(lobby_list& lobby) {
  while (auto evOpt = PopEventLobby(lobby)) {
    auto [event, playerId] = evOpt.value();

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
