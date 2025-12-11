#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <vector>

#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"
#include "network/ServerNetworkManager.hpp"

class ServerGame {
 public:
  ServerGame();
  bool Initialize(uint16_t tcpPort, uint16_t udpPort);
  void Run();
  void Shutdown();

 private:
  ServerNetworkManager networkManager;
  Decoder decode;

  std::queue<std::tuple<Event, uint16_t>> eventQueue;
  std::queue<std::tuple<Action, uint16_t>> actionQueue;
  std::mutex queueMutex;

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
  
  //std::optional<std::tuple<Event, uint16_t>> ServerGame::PopEvent();
  //void SendAction(std::tuple<Action, uint16_t>);
};
