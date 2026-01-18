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
#include "Collision/MapCollisionSystem.hpp"
#include "Helpers/EntityHelper.hpp"
#include "Movement/Movement.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "components/BossPart.hpp"
#include "components/Force.hpp"
#include "components/TileMap.hpp"
#include "ecs/Registry.hpp"
#include "network/DataMask.hpp"
#include "scene/Scene.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/ChargedShoot.hpp"
#include "systems/ForceCtrl.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/MapGenerator.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"

ServerGame::ServerGame() : serverRunning(true) {
  SetupDecoder(decode);
  SetupEncoder(encode);
#ifdef _WIN32
  DLLoader<INetworkManager> loader("../src/build/libnetwork_server.dll",
                                   "EntryPointLib");
#else
  DLLoader<INetworkManager> loader("../src/build/libnetwork_server.so",
                                   "EntryPointLib");
#endif
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

  if (!l->m_engine.Initialize("Server Lobby", 0, 0)) {
    std::cerr << "Failed to initialize lobby engine!" << std::endl;
    return;
  }

#ifdef _WIN32
  if (!l->m_engine.GetSceneManager().LoadSceneModule("rtype", "../../Client/src/scenes/libscene_rtypescene.dll")) {
    std::cerr << "Failed to load game scene!" << std::endl;
    return;
  }
#else
  if (!l->m_engine.GetSceneManager().LoadSceneModule("rtype", "../../Client/src/scenes/libscene_rtypescene.so")) {
    std::cerr << "Failed to load game scene!" << std::endl;
    return;
  }
#endif

  l->m_gameScene = l->m_engine.GetSceneManager().GetCurrentScene();
  if (l->m_gameScene) {
    std::cout << "scene loaded:" << l->m_gameScene->GetName() << std::endl;
  }

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
    auto& lobby = *it;

    for (auto pIt = lobby->players_list.begin();
         pIt != lobby->players_list.end(); ++pIt) {
      if (std::get<0>(*pIt) == playerId) {
        lobby->players_list.erase(pIt);
        lobby->nb_player--;
        found = true;
        break;
      }
    }

    if (found) {
      if (lobby->nb_player == 0) {
        lobby->gameRuning = false;
        if (lobby->gameThread.joinable()) {
          lobby->gameThread.join();
        }
        it = lobbys.erase(it);
      } else {
        if (lobby->host_id == playerId) {
          lobby->host_id = std::get<0>(lobby->players_list[0]);
        }
        SendLobbyUpdate(*lobby);
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

  lobby_list* l;

  for (auto& lobby : lobbys) {
    uint16_t nbPlayerReady = 0;
    bool found = false;
    bool isSpectate = false;

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
        isSpectate = true;
        l = lobby.get();
      }
    }

    if (found) {
      SendLobbyUpdate(*lobby);

      if (lobby->gameRuning && isReady) {
        size_t playerIndex = 0;
        for (size_t i = 0; i < lobby->players_list.size(); ++i) {
          if (std::get<0>(lobby->players_list[i]) == playerId) {
            playerIndex = i;
            break;
          }
        }

        float spawnY = 200.0f + (playerIndex % 4) * 100.0f;

        if (isSpectate) {
          SendMapToClients(playerId, *l);
          Action startAc;
          GameStart gs;
          gs.playerSpawnX = 200.0f;
          gs.playerSpawnY = spawnY;
          gs.scrollSpeed = 0;
          startAc.type = ActionType::GAME_START;
          startAc.data = gs;
          SendAction(std::make_tuple(startAc, playerId, nullptr));
        }
        lobby->lastStates.erase(playerId);
        lobby->playerStateCount.erase(playerId);

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

void ServerGame::HandleLobbyKick(uint16_t playerId, uint16_t playerKickId) {
  lobby_list* lobby = FindPlayerLobby(playerId);
  if (!lobby) return;

  if (playerId != lobby->host_id) return;
  Action ac;
  ac.type = ActionType::LOBBY_KICK;
  LobbyKick resp;
  resp.playerId = playerKickId;
  ac.data = resp;
  SendAction(std::make_tuple(ac, playerKickId, nullptr));
  RemovePlayerFromLobby(playerKickId);
  SendLobbyUpdate(*lobby);
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

void ServerGame::HandleClientLeave(uint16_t playerId) {
  std::cout << "[SERVER] Handling cleanup for client " << playerId << std::endl;

  lobby_list* lobby = FindPlayerLobby(playerId);

  if (lobby) {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    auto itEntity = lobby->m_players.find(playerId);
    if (itEntity != lobby->m_players.end()) {
      if (lobby->registry.is_entity_valid(itEntity->second)) {
        lobby->registry.kill_entity(itEntity->second);
      }

      lobby->m_players.erase(itEntity);
    }

    for (auto specIt = lobby->spectate.begin(); specIt != lobby->spectate.end();
         ++specIt) {
      if (std::get<0>(*specIt) == playerId) {
        lobby->spectate.erase(specIt);
        lobby->nb_player--;
        break;
      }
    }
  }
  RemovePlayerFromLobby(playerId);
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
        HandleLobbyKick(playerId, d.playerId);
        break;
      }
      case EventType::MESSAGE:
        HandleLobbyMessage(playerId, ev);
        break;

      case EventType::CLIENT_LEAVE:
        HandleClientLeave(playerId);
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
      auto m_players = lobby->m_gameScene->GetPlayers();
      auto it = m_players.find(client_id);
      if (it != m_players.end()) {
        Entity playerEntity = it->second;
        lobby->m_engine.GetRegistry().kill_entity(playerEntity);
        m_players.erase(it);
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

void ServerGame::StartGame(lobby_list& lobby) {
  // if (!lobby.m_gameScene) {
  //   std::cerr << "No game scene loaded!" << std::endl;
  //   return;
  // }

  SceneData& data = lobby.m_engine.GetSceneData();
  data.Set("players_list", lobby.players_list);
  data.Set("difficulty", lobby.difficulty);

  lobby.m_engine.ChangeScene("rtype");
  Scene* currentScene = lobby.m_engine.GetCurrentScene();
  if (currentScene) {
    currentScene->OnExit();
    currentScene->OnEnter();
  }

  std::cout << "Lobby full! Starting the game!!!!\n";
  lobby.gameThread = std::thread(&ServerGame::GameLoop, this, std::ref(lobby));
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
  lobby.lastStates.clear();
  lobby.playerStateCount.clear();
  lobby.gameRuning = false;
  std::cout << "game ended!!" << std::endl;
}

void ServerGame::CheckGameEnded(lobby_list& lobby) {
  lobby.m_engine.ChangeScene("rtype");
  auto m_players = lobby.m_engine.GetCurrentScene()->GetPlayers();
  std::cout << "got players" << std::endl;
  SceneData& data = lobby.m_engine.GetSceneData();
  Registry& registry = lobby.m_engine.GetRegistry();

  int validPlayers = 0;
  for (auto& [id, entity] : m_players) {
    if (registry.is_entity_valid(entity)) {
      validPlayers += 1;
    }
  }

  if (validPlayers <= 0) {
    EndGame(lobby);
    return;
  }
  if (data.Has("game_ended")) {
    EndGame(lobby);
  }
}

void ServerGame::GameLoop(lobby_list& lobby) {
  for (int i = 0; i < 50 && lobby.gameRuning; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  const auto frameDuration = std::chrono::milliseconds(100);
  auto lastTime = std::chrono::steady_clock::now();

  if (lobby.gameRuning) {
    size_t playerIndex = 0;
    SendMapToLobby(lobby);
    for (auto& [playerId, ready, _] : lobby.players_list) {
      float spawnY = 200.0f + (playerIndex % 4) * 100.0f;

      Action ac;
      GameStart g;
      g.playerSpawnX = 200.0f;
      g.playerSpawnY = spawnY;
      g.scrollSpeed = 0;
      ac.type = ActionType::GAME_START;
      ac.data = g;
      SendAction(std::make_tuple(ac, playerId, nullptr));

      //  auto currentState =
      //       std::make_shared<GameState>(BuildCurrentState(lobby));
      //   ProcessAndSendState(playerId, lobby, currentState);
      playerIndex++;
    }
  }

  std::cout << "game starting" << std::endl;

  while (lobby.gameRuning) {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    ReceivePlayerInputs(lobby);
    lobby.m_engine.Update(deltaTime);
    SendWorldStateToClients(lobby);
    CheckGameEnded(lobby);
    std::cout << "lobby" << std::endl;

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
 Scene* currentScene = lobby.m_engine.GetCurrentScene();
  if (currentScene) {
    currentScene->OnExit();
  }
  
  lobby.m_engine.GetRegistry().clear_all_entities();
  
  SceneData& data = lobby.m_engine.GetSceneData();
  data.Remove("game_ended");
  data.Remove("victory");
  data.Remove("game_state");
  data.Remove("force_state");
  data.Remove("last_input_event");
  data.Remove("last_input_player");
  
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
  lobby.mapSent = false;
  lobby.lastStates.clear();
  lobby.playerStateCount.clear();

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

    // switch (event.type) {
    //   case EventType::PLAYER_INPUT: {
    //     const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);

    //     auto& players = lobby.registry.get_components<PlayerEntity>();
    //     auto& states = lobby.registry.get_components<InputState>();

    //     for (auto&& [player, state] : Zipper(players, states)) {
    //       if (player.player_id != playerId) continue;

    //       state.moveLeft = input.left;
    //       state.moveRight = input.right;
    //       state.moveDown = input.down;
    //       state.moveUp = input.up;
    //       state.action1 = (input.fire == 1);
    //       state.action2 = (input.fire == 2);
    //       break;
    //     }
    //     break;
    //   }

    //   default:
    //     break;
    // }
    SceneData& data = lobby.m_engine.GetSceneData();
    data.Set("last_input_event", event);
    data.Set("last_input_player", playerId);
  }
}

void ServerGame::UpdateGameState(lobby_list& lobby, float deltaTime) {}

void ServerGame::SendMapToLobby(lobby_list& lobby) {
  if (lobby.mapSent) return;
  SceneData& data = lobby.m_engine.GetSceneData();

  if (data.Has("send_map")) {
    MapData mapdata = data.Get<MapData>("send_map");
    SendAction(
        std::make_tuple(Action{ActionType::SEND_MAP, mapdata}, 0, &lobby));
    lobby.mapSent = true;

    std::cout << "[Server] Map sent to clients (" << mapdata.width << "x"
              << mapdata.height << " tiles, " << mapdata.tiles.size()
              << " bytes)" << std::endl;
  }
}

void ServerGame::SendMapToClients(uint16_t playerId, lobby_list& lobby) {
  if (lobby.mapSent) return;

  SceneData& data = lobby.m_engine.GetSceneData();

  if (data.Has("send_map")) {
    MapData mapdata = data.Get<MapData>("send_map");
    SendAction(
        std::make_tuple(Action{ActionType::SEND_MAP, mapdata}, 0, &lobby));
    lobby.mapSent = true;

    std::cout << "[Server] Map sent to clients (" << mapdata.width << "x"
              << mapdata.height << " tiles, " << mapdata.tiles.size()
              << " bytes)" << std::endl;
  }
}

GameState ServerGame::CalculateDelta(const GameState& last,
                                     const GameState& current) {
  GameState diff;

  for (const auto& currP : current.players) {
    auto it = std::find_if(
        last.players.begin(), last.players.end(),
        [&](const PlayerState& p) { return p.playerId == currP.playerId; });

    if (it == last.players.end()) {
      diff.players.push_back(currP);
    } else {
      PlayerState deltaP;
      deltaP.playerId = currP.playerId;
      deltaP.mask = 0;

      if (std::abs(currP.posX - it->posX) > 0.01f) {
        deltaP.posX = currP.posX;
        deltaP.mask |= M_POS_X;
      }
      if (std::abs(currP.posY - it->posY) > 0.01f) {
        deltaP.posY = currP.posY;
        deltaP.mask |= M_POS_Y;
      }
      if (currP.hp != it->hp) {
        deltaP.hp = currP.hp;
        deltaP.mask |= M_HP;
      }
      if (currP.shield != it->shield) {
        deltaP.shield = currP.shield;
        deltaP.mask |= M_SHIELD;
      }
      if (currP.weapon != it->weapon) {
        deltaP.weapon = currP.weapon;
        deltaP.mask |= M_WEAPON;
      }
      if (currP.state != it->state) {
        deltaP.state = currP.state;
        deltaP.mask |= M_STATE;
      }
      if (currP.sprite != it->sprite) {
        deltaP.sprite = currP.sprite;
        deltaP.mask |= M_SPRITE;
      }
      if (currP.score != it->score) {
        deltaP.score = currP.score;
        deltaP.mask |= M_SCORE;
      }

      if (deltaP.mask != 0) {
        diff.players.push_back(deltaP);
      }
    }
  }

  for (const auto& lastP : last.players) {
    if (std::none_of(current.players.begin(), current.players.end(),
                     [&](const PlayerState& p) {
                       return p.playerId == lastP.playerId;
                     })) {
      PlayerState deleteP;
      deleteP.playerId = lastP.playerId;
      deleteP.mask = M_DELETE;
      deleteP.state = 0;
      diff.players.push_back(deleteP);
    }
  }

  for (const auto& currE : current.enemies) {
    auto it = std::find_if(
        last.enemies.begin(), last.enemies.end(),
        [&](const EnemyState& e) { return e.enemyId == currE.enemyId; });

    if (it == last.enemies.end()) {
      diff.enemies.push_back(currE);
    } else {
      EnemyState deltaE;
      deltaE.enemyId = currE.enemyId;
      deltaE.mask = 0;

      if (std::abs(currE.posX - it->posX) > 0.01f) {
        deltaE.posX = currE.posX;
        deltaE.mask |= M_POS_X;
      }
      if (std::abs(currE.posY - it->posY) > 0.01f) {
        deltaE.posY = currE.posY;
        deltaE.mask |= M_POS_Y;
      }
      if (currE.hp != it->hp) {
        deltaE.hp = currE.hp;
        deltaE.mask |= M_HP;
      }
      if (currE.enemyType != it->enemyType) {
        deltaE.enemyType = currE.enemyType;
        deltaE.mask |= M_TYPE;
      }
      if (currE.state != it->state) {
        deltaE.state = currE.state;
        deltaE.mask |= M_STATE;
      }
      if (currE.direction != it->direction) {
        deltaE.direction = currE.direction;
        deltaE.mask |= M_DIR;
      }

      if (deltaE.mask != 0) {
        diff.enemies.push_back(deltaE);
      }
    }
  }

  for (const auto& lastE : last.enemies) {
    if (std::none_of(
            current.enemies.begin(), current.enemies.end(),
            [&](const EnemyState& e) { return e.enemyId == lastE.enemyId; })) {
      EnemyState deleteE;
      deleteE.enemyId = lastE.enemyId;
      deleteE.mask = M_DELETE;
      deleteE.hp = 0;
      diff.enemies.push_back(deleteE);
    }
  }

  for (const auto& currPr : current.projectiles) {
    auto it = std::find_if(last.projectiles.begin(), last.projectiles.end(),
                           [&](const ProjectileState& pr) {
                             return pr.projectileId == currPr.projectileId;
                           });

    if (it == last.projectiles.end()) {
      diff.projectiles.push_back(currPr);
    } else {
      ProjectileState deltaPr;
      deltaPr.projectileId = currPr.projectileId;
      deltaPr.mask = 0;

      if (std::abs(currPr.posX - it->posX) > 0.01f) {
        deltaPr.posX = currPr.posX;
        deltaPr.mask |= M_POS_X;
      }
      if (std::abs(currPr.posY - it->posY) > 0.01f) {
        deltaPr.posY = currPr.posY;
        deltaPr.mask |= M_POS_Y;
      }
      if (currPr.damage != it->damage) {
        deltaPr.damage = currPr.damage;
        deltaPr.mask |= M_DAMAGE;
      }

      if (deltaPr.mask != 0) {
        diff.projectiles.push_back(deltaPr);
      }
    }
  }

  for (const auto& lastPr : last.projectiles) {
    if (std::none_of(current.projectiles.begin(), current.projectiles.end(),
                     [&](const ProjectileState& pr) {
                       return pr.projectileId == lastPr.projectileId;
                     })) {
      ProjectileState deletePr;
      deletePr.projectileId = lastPr.projectileId;
      deletePr.mask = M_DELETE;
      deletePr.damage = 0;
      diff.projectiles.push_back(deletePr);
    }
  }

  return diff;
}

void ServerGame::ProcessAndSendState(uint16_t playerId, lobby_list& lobby) {
  auto& lastStatePtr = lobby.lastStates[playerId];
  auto& stateCount = lobby.playerStateCount[playerId];
  bool isFirstPacket = false;

  if (stateCount < 3) {
    isFirstPacket = true;
    stateCount++;
  }

  GameState deltaState;
  SceneData& data = lobby.m_engine.GetSceneData();

  if (data.Has("game_state")) {
    deltaState = data.Get<GameState>("game_state");
  } else {
    return;
  }
  if (isFirstPacket) {
    uint16_t fullMaskPlayer = M_POS_X | M_POS_Y | M_HP | M_STATE | M_SHIELD |
                              M_WEAPON | M_SPRITE | M_SCORE;

    uint16_t fullMaskEnemy =
        M_POS_X | M_POS_Y | M_HP | M_STATE | M_TYPE | M_DIR;
    for (auto& e : deltaState.enemies) {
      e.mask = fullMaskEnemy;
    }

    uint16_t fullMaskProj = M_POS_X | M_POS_Y | M_DAMAGE | M_TYPE | M_OWNER;
    for (auto& pr : deltaState.projectiles) {
      pr.mask = fullMaskProj;
    }

  } else {
    if (!lastStatePtr) {
      lastStatePtr = std::make_shared<GameState>();
    }
    deltaState =
        CalculateDelta(*lastStatePtr, data.Get<GameState>("game_state"));
    lobby.lastStates[playerId] =
        std::make_shared<GameState>(data.Get<GameState>("game_state"));
  }

  if (isFirstPacket || !deltaState.players.empty() ||
      !deltaState.enemies.empty() || !deltaState.projectiles.empty()) {
    Action ac;
    ac.type = ActionType::GAME_STATE;
    ac.data = deltaState;

    NetworkMessage msg;
    msg.client_id = playerId;
    networkManager->SendTo(msg, ac);
  }
}

void ServerGame::SendWorldStateToClients(lobby_list& lobby) {
  SceneData& data = lobby.m_engine.GetSceneData();

  if (data.Has("force_state")) {
    ForceState fs = data.Get<ForceState>("force_state");
    SendAction(std::make_tuple(Action{ActionType::FORCE_STATE, fs}, 0, &lobby));
  }

  for (auto& [playerId, ready, name] : lobby.players_list) {
    if (playerId == 0) continue;
    ProcessAndSendState(playerId, lobby);
  }

  for (auto& [playerId, ready, name] : lobby.spectate) {
    if (playerId == 0) continue;
    ProcessAndSendState(playerId, lobby);
  }
}
