#pragma once
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "components/Levels.hpp"
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
struct lobby_list {
  uint16_t lobby_id;
  uint16_t host_id;
  std::string name = "";
  std::string mdp = "";
  uint8_t difficulty = 1;
  uint8_t nb_player;
  uint8_t max_players = 4;
  std::vector<std::tuple<uint16_t, bool, std::string>> players_list;
  bool players_ready = false;
  bool gameRuning = false;
  bool hasPassword = false;
  std::vector<LevelComponent> levelsData;
  int currentLevelIndex = 0;
  bool waitingForNextLevel = false;
  float levelTransitionTimer = 0.0f;
  Entity currentLevelEntity;
  Registry registry;
  std::unordered_map<uint16_t, Entity> m_players;
  std::thread gameThread;
};
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
  std::unique_ptr<INetworkManager>
      networkManager;  ///< Network communication manager
  Decoder decode;      ///< Decoder for incoming network messages
  Encoder encode;      ///< Encoder for outgoing network messages
  Registry registry;   ///< ECS registry for game entities
  std::unordered_map<uint16_t, Entity>
      m_players;          ///< Map of player IDs to their entities
  std::mutex queueMutex;  ///< Mutex for thread-safe queue access

  std::vector<std::unique_ptr<lobby_list>> lobbys;
  uint16_t nextLobbyId = 1;
  const float TIME_BETWEEN_LEVELS = 5.0f;

  std::queue<std::tuple<Event, uint16_t>>
      eventQueue;  ///< Queue of incoming events from clients
  std::queue<std::tuple<Action, uint16_t, lobby_list*>>
      actionQueue;  ///< Queue of actions to send to clients

  void CreateLobby(uint16_t playerId, std::string name, std::string playerName,
                   std::string mdp, uint8_t difficulty, uint8_t Maxplayer);
  void JoinLobby(uint16_t playerId, std::string playerName, uint16_t lobbyId,
                 std::string mdp);
  void HandlePayerReady(uint16_t playerId);
  void ResetLobbyReadyStatus(lobby_list& lobby);
  void ClearLobbyForRematch(lobby_list& lobby);

  void HandleLobbyCreate(uint16_t playerId, Event& ev);
  void HandleLobbyJoinRequest(uint16_t playerId, Event& ev);
  void HandleLobbyListRequest(uint16_t playerId);
  void HandleLobbyLeave(uint16_t playerId);
  void SendLobbyUpdate(lobby_list& lobby);
  void RemovePlayerFromLobby(uint16_t playerId);
  lobby_list* FindPlayerLobby(uint16_t playerId);

  /**
   * @brief Receive and process player input events
   */
  void ReceivePlayerInputs(lobby_list& lobby);
  /**
   * @brief Update game state based on elapsed time
   * @param deltaTime Time elapsed since last update
   */
  void UpdateGameState(lobby_list& lobby, float deltaTime);
  /**
   * @brief Send current world state to all connected clients
   */
  void SendWorldStateToClients(lobby_list& lobby);
  std::mutex lobbyMutex;  ///< Mutex for thread-safe lobby access

  bool serverRunning;  ///< Server running state flag

  /**
   * @brief Setup network event callbacks
   */
  void SetupNetworkCallbacks();

  /**
   * @brief Handle player authentication
   * @param playerId ID of the player to authenticate
   */
  void HandleAuth(uint16_t playerId, Event& ev);
  /**
   * @brief Start the game when all players are ready
   */
  void StartGame(lobby_list& lobby_list);
  /**
   * @brief Initialize the game world with entities
   */
  void InitWorld(lobby_list& lobby);
  /**
   * @brief Main game loop running in separate thread
   */
  void GameLoop(lobby_list& lobby);
  /**
   * @brief Check if game end conditions are met
   */
  void CheckGameEnded(lobby_list& lobby);
  /**
   * @brief End the game and cleanup
   */
  void EndGame(lobby_list& lobby);
  /**
   * @brief Pop an event from the event queue
   * @return Optional tuple containing event and client ID
   */
  std::optional<std::tuple<Event, uint16_t>> PopEvent();

  /**
   * @brief Send an action to a specific client
   * @param action Tuple containing action and client ID
   */
  void SendAction(std::tuple<Action, uint16_t, lobby_list*>);

  /**
   * @brief Send queued packets to clients
   */
  void SendPacket();
};
