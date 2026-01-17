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
#include "physics/Physics2D.hpp"
#include "dynamicLibLoader/DLLoader.hpp"
#include "ecs/Registry.hpp"
#include "systems/BoundsSystem.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WaveSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "scene/Scene.hpp"

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

  if (!l->m_engine.Initialize("Server Lobby", 0, 0)) {
    std::cerr << "Failed to initialize lobby engine!" << std::endl;
    return;
  }

  if (!l->m_engine.GetSceneManager().LoadSceneModule("rtype", "../../Client/src/scenes/libscene_rtypescene.so")) {
    std::cerr << "Failed to load game scene!" << std::endl;
    return;
  }

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
      case EventType::LOGIN_REQUEST:
        std::cout << "[Network] LOGIN_REQUEST from " << playerId << std::endl;
        break;

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

bool ServerGame::Initialize(uint16_t tcpPort, uint16_t udpPort, int diff, const std::string& host) {
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

  std::cout << "Lobby full! Starting the game!!!!\n";
  lobby.gameThread =
      std::thread(&ServerGame::GameLoop, this, std::ref(lobby));
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
  auto m_players = lobby.m_engine.GetCurrentScene()->GetPlayers();
  std::cout << "got players" << std::endl;
  Registry& registry = lobby.m_engine.GetRegistry();
  
  int validPlayers = 0;
  for (auto& [id, entity] : m_players) {
    if (registry.is_entity_valid(entity)) {
      validPlayers += 1;
    }
  }
  
  if (validPlayers <= 0) {
    EndGame(lobby);
  }
}

void ServerGame::GameLoop(lobby_list& lobby) {

  for (int i = 0; i < 50 && lobby.gameRuning; ++i) {
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
      if (protocol == 0) {
        networkManager->BroadcastLobbyUDP(msg, lobby->players_list);
        networkManager->BroadcastLobbyUDP(msg, lobby->spectate);
      }
      if (protocol == 2) {
        networkManager->BroadcastLobbyTCP(msg, lobby->players_list);
        networkManager->BroadcastLobbyTCP(msg, lobby->spectate);
      }
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

void ServerGame::ClearLobbyForRematch(lobby_list& lobby) {
  // lobby.registry = Registry{};
  // lobby.m_players.clear();
  // lobby.currentLevelIndex = 0;
  // lobby.waitingForNextLevel = false;
  // lobby.levelTransitionTimer = 0.0f;

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
  std::cout << "before main loop"<< std::endl;

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
  std::cout << "server "<< std::endl;
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
    //       state.action1 = input.fire;
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

void ServerGame::UpdateGameState(lobby_list& lobby, float deltaTime) {
}

void ServerGame::SendWorldStateToClients(lobby_list& lobby) {
  SceneData& data = lobby.m_engine.GetSceneData();

  if (data.Has("game_state")) {
    GameState gs = data.Get<GameState>("game_state");
    SendAction(std::make_tuple(Action{ActionType::GAME_STATE, gs}, 0, &lobby));
  }
}
