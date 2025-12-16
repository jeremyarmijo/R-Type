#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <vector>

#include "ecs/Registry.hpp"
#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"
#include "network/ServerNetworkManager.hpp"

class ServerGame {
 public:
  ServerGame();
  bool Initialize(uint16_t tcpPort, uint16_t udpPort, int diff);
  void Run();
  void Shutdown();

 private:
  ServerNetworkManager networkManager;
  Decoder decode;
  Encoder encode;
  Registry registry;
  std::queue<std::tuple<Event, uint16_t>> eventQueue;
  std::queue<std::tuple<Action, uint16_t>> actionQueue;
  std::unordered_map<uint16_t, Entity> m_players;
  int difficulty;
  std::mutex queueMutex;
  void ReceivePlayerInputs();
  void UpdateGameState(float deltaTime);
  void SendWorldStateToClients();

  std::vector<uint16_t> lobbyPlayers;
  std::mutex lobbyMutex;

  bool serverRunning;
  bool gameStarted;

  std::thread gameThread;

  void SetupNetworkCallbacks();
  void HandleAuth(uint16_t playerId);
  void StartGame();
  void InitWorld();
  void GameLoop();
  bool CheckGameEnded();
  void EndGame();

  std::optional<std::tuple<Event, uint16_t>> PopEvent();
  void SendAction(std::tuple<Action, uint16_t>);

  void SendPacket();
};
