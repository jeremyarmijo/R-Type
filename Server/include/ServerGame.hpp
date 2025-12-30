#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <vector>
#include <string>
#include <unordered_map>
#include "ecs/Registry.hpp"
#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"
#include "network/ServerNetworkManager.hpp"

/**
 * @class ServerGame
 * @brief Main server game logic manager
 *
 * Handles the core game loop, player management, network communication,
 * and game state synchronization for the R-Type server.
 */
class ServerGame {
 public:
  /**
   * @brief Construct a new ServerGame object
   */
  ServerGame();
  bool Initialize(uint16_t tcpPort, uint16_t udpPort, int diff,
                  const std::string& host = "0.0.0.0");
  void Run();

  /**
   * @brief Shutdown the server and cleanup resources
   */
  void Shutdown();

 private:
  ServerNetworkManager networkManager;  ///< Network communication manager
  Decoder decode;     ///< Decoder for incoming network messages
  Encoder encode;     ///< Encoder for outgoing network messages
  Registry registry;  ///< ECS registry for game entities
  std::queue<std::tuple<Event, uint16_t>>
      eventQueue;  ///< Queue of incoming events from clients
  std::queue<std::tuple<Action, uint16_t>>
      actionQueue;  ///< Queue of actions to send to clients
  std::unordered_map<uint16_t, Entity>
      m_players;          ///< Map of player IDs to their entities
  int difficulty;         ///< Game difficulty setting
  std::mutex queueMutex;  ///< Mutex for thread-safe queue access

  /**
   * @brief Receive and process player input events
   */
  void ReceivePlayerInputs();

  /**
   * @brief Update game state based on elapsed time
   * @param deltaTime Time elapsed since last update
   */
  void UpdateGameState(float deltaTime);

  /**
   * @brief Send current world state to all connected clients
   */
  void SendWorldStateToClients();

  std::vector<uint16_t> lobbyPlayers;  ///< List of players in the lobby
  std::mutex lobbyMutex;               ///< Mutex for thread-safe lobby access

  bool serverRunning;  ///< Server running state flag
  bool gameStarted;    ///< Game started state flag

  std::thread gameThread;  ///< Main game loop thread

  /**
   * @brief Setup network event callbacks
   */
  void SetupNetworkCallbacks();

  /**
   * @brief Handle player authentication
   * @param playerId ID of the player to authenticate
   */
  void HandleAuth(uint16_t playerId);

  /**
   * @brief Start the game when all players are ready
   */
  void StartGame();

  /**
   * @brief Initialize the game world with entities
   */
  void InitWorld();

  /**
   * @brief Main game loop running in separate thread
   */
  void GameLoop();

  /**
   * @brief Check if game end conditions are met
   */
  void CheckGameEnded();

  /**
   * @brief End the game and cleanup
   */
  void EndGame();

  /**
   * @brief Pop an event from the event queue
   * @return Optional tuple containing event and client ID
   */
  std::optional<std::tuple<Event, uint16_t>> PopEvent();

  /**
   * @brief Send an action to a specific client
   * @param action Tuple containing action and client ID
   */
  void SendAction(std::tuple<Action, uint16_t>);

  /**
   * @brief Send queued packets to clients
   */
  void SendPacket();
};
