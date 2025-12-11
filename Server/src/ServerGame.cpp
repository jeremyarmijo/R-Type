#include "include/ServerGame.hpp"
#include <iostream>

ServerGame::ServerGame() : serverRunning(true), gameStarted(false) {
  SetupDecoder(decode);
}

void ServerGame::HandleAuth(uint16_t playerId) {
  if (std::find(lobbyPlayers.begin(), lobbyPlayers.end(), playerId) !=
      lobbyPlayers.end())
    return;

  lobbyPlayers.push_back(playerId);
  std::cout << "Player " << playerId << " joined the lobby ("
            << lobbyPlayers.size() << "/4)" << std::endl;

  if (lobbyPlayers.size() == 4 && !gameStarted) {
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

  networkManager.SetConnectionCallback([this](uint32_t client_id) {
    std::cout << "Client " << client_id << " connected!" << std::endl;
  });

  networkManager.SetDisconnectionCallback([this](uint32_t client_id) {
    std::cout << "Client " << client_id << " disconnected!" << std::endl;
    std::lock_guard<std::mutex> lock(lobbyMutex);
    lobbyPlayers.erase(
        std::remove(lobbyPlayers.begin(), lobbyPlayers.end(), client_id),
        lobbyPlayers.end());
  });
}

bool ServerGame::Initialize(uint16_t tcpPort, uint16_t udpPort) {
  if (!networkManager.Initialize(tcpPort, udpPort)) {
    std::cerr << "Failed to initialize network manager" << std::endl;
    return false;
  }
  SetupNetworkCallbacks();
  return true;
}

void ServerGame::InitWorld() {
    /*// 1. On crée le joueur
    Vector2 playerStartPos{100.f, 200.f};
    Entity player = createPlayer(registry, playerStartPos);

    // 2. On crée un ennemi
    Vector2 enemyPos{400.f, 200.f};
    Entity enemy = createEnemy(registry, EnemyType::Basic, enemyPos);

    // 3. On peut créer un boss
    Vector2 bossPos{800.f, 100.f};
    Entity boss = createBoss(registry, BossType::BigShip, bossPos);*/
}

void ServerGame::StartGame() {
  gameStarted = true;
  std::cout << "Lobby full! Starting the game!!!!\n";
  gameThread = std::thread(&ServerGame::GameLoop, this);
}

/*std::optional<std::tuple<Event, uint16_t>> ServerGame::PopEvent() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (eventQueue.empty()) {
        return std::nullopt;
    }
    auto res = eventQueue.front();
    eventQueue.pop();
    return res;
}*/

/*void ServerGame::SendAction(std::tuple<Action, uint16_t> ac) {
    std::lock_guard<std::mutex> lock(queueMutex);
    actionQueue.push(std::move(ac));
}*/

void ServerGame::GameLoop() {
  InitWorld();

  const auto frameDuration = std::chrono::milliseconds(16);
  auto lastTime = std::chrono::steady_clock::now();

  while (serverRunning) {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    std::cout << "GAMEEEEEEE!!!!\n";
    /*receive_player_inputs();      // 1. Récupérer les inputs depuis le serveur
    update_game_state(deltaTime); // 2. Déplacement, ticks
    run_systems();                // 3. Collisions, IA
    cleanup_entities();           // 4. Je clean les entités mortes
    send_updates_to_clients();    // 5. Envoyer l’état aux clients*/

    auto frameTime = std::chrono::steady_clock::now() - currentTime;
    if (frameTime < frameDuration) {
      std::this_thread::sleep_for(frameDuration - frameTime);
    }
  }
}

void ServerGame::Run() {
  while (serverRunning) {
    networkManager.Update();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void ServerGame::Shutdown() {
  serverRunning = false;
  if (gameThread.joinable()) gameThread.join();

  networkManager.Shutdown();
}
