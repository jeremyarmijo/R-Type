#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "db/IDatabase.hpp"
#include "network/ClientManager.hpp"
#include "network/INetworkManager.hpp"
#include "network/TCPServer.hpp"
#include "network/UDPServer.hpp"

struct ConnectionEvent {
  enum Type { CONNECTED, DISCONNECTED };  ///< Event type
  Type type;                              ///< The type of connection event
  uint32_t client_id;                     ///< Client identifier
};

/**
 * @class ServerNetworkManager
 * @brief Manages all server-side network operations
 *
 * Handles TCP and UDP communication, client connections/disconnections,
 * and message routing between the server and clients.
 */
class ServerNetworkManager : public INetworkManager {
 public:
  /**
   * @brief Construct a new ServerNetworkManager
   */
  ServerNetworkManager();

  /**
   * @brief Destroy the ServerNetworkManager and cleanup resources
   */
  ~ServerNetworkManager() override;

  bool Initialize(uint16_t tcp_port, uint16_t udp_port,
                  const std::string& host = "0.0.0.0") override;
  void Shutdown() override;

  /**
   * @brief Send a message to a specific client
   * @param msg Network message to send
   * @param sendUdp If true, use UDP; otherwise use TCP
   */
  void SendTo(const NetworkMessage& msg, bool sendUdp) override;

  /**
   * @brief Broadcast a message to all clients via UDP
   * @param msg Network message to broadcast
   */
  void BroadcastLobbyUDP(
      const NetworkMessage& msg,
      std::vector<std::tuple<uint16_t, bool, std::string>>&) override;

  /**
   * @brief Broadcast a message to all clients via TCP
   * @param msg Network message to broadcast
   */
  void BroadcastLobbyTCP(
      const NetworkMessage& msg,
      std::vector<std::tuple<uint16_t, bool, std::string>>&) override;

  /**
   * @brief Process incoming messages and connection events
   */
  void Update() override;

  /**
   * @brief Set callback for incoming messages
   * @param callback Function to call when message is received
   */
  void SetMessageCallback(MessageCallback callback) override;

  /**
   * @brief Set callback for client connections
   * @param callback Function to call when client connects
   */
  void SetConnectionCallback(ConnectionCallback callback) override;

  /**
   * @brief Set callback for client disconnections
   * @param callback Function to call when client disconnects
   */
  void SetDisconnectionCallback(DisconnectionCallback callback) override;

  /**
   * @brief Get user information from database
   * @param username Username to search for
   * @param password Output parameter for user's password
   * @param score Output parameter for user's score
   * @return true if user found, false otherwise
   */
  bool GetUser(const std::string& username, std::string& password, int& score) override;

  /**
   * @brief Add a new user and update if it exists in the database
   * @param username Username for the new user
   * @param password Password for the new user
   * @param score Initial score for the new user
   * @return true if user added successfully, false otherwise
   */
  bool AddUser(const std::string& username, const std::string& password, int score) override;

  /**
   * @brief Set game started state
   * @param strated True if game has started, false otherwise
   */
  void SetGameStarted(bool strated) { GameStarted = strated; }

 private:
  /**
   * @brief ASIO I/O context thread function
   */
  void IOThreadFunc();

  /**
   * @brief Check for timed out clients and disconnect them
   */
  void CheckClientTimeouts();

  /**
   * @brief Handle incoming UDP data
   * @param data Received data bytes
   * @param sender Sender endpoint
   */
  void OnReceive(const std::vector<uint8_t>& data,
                 const asio::ip::udp::endpoint& sender);
  void OnReceiveTCP(uint32_t client_id, const std::vector<uint8_t>& data);

  /**
   * @brief Handle TCP login event
   * @param client_id Client identifier
   * @param username Client username
   * @param tcp_endpoint Client TCP endpoint
   */
  void OnTCPLogin(uint32_t client_id, const std::string& username,
                  const asio::ip::tcp::endpoint& tcp_endpoint);

  /**
   * @brief Handle TCP disconnect event
   * @param client_id Client identifier
   */
  void OnTCPDisconnect(uint32_t client_id);

  std::unique_ptr<IDatabase> db;
  bool GameStarted = false;      ///< Game started state flag
  asio::io_context io_context_;  ///< ASIO I/O context for async operations
  std::unique_ptr<asio::io_context::work>
      work_guard_;  ///< Work guard to keep I/O context running
  std::unique_ptr<TCPServer> tcp_server_;  ///< TCP server instance
  std::unique_ptr<UDPServer> udp_server_;  ///< UDP server instance
  std::thread io_thread_;                  ///< Thread for running I/O context

  ClientManager client_manager_;  ///< Manager for connected clients

  std::queue<NetworkMessage>
      incoming_messages_;   ///< Queue of incoming messages
  std::mutex queue_mutex_;  ///< Mutex for message queue access

  std::queue<ConnectionEvent>
      connection_events_;    ///< Queue of connection events
  std::mutex events_mutex_;  ///< Mutex for events queue access

  MessageCallback message_callback_;        ///< Callback for incoming messages
  ConnectionCallback connection_callback_;  ///< Callback for client connections
  DisconnectionCallback
      disconnection_callback_;  ///< Callback for client disconnections

  std::unique_ptr<asio::steady_timer>
      timeout_timer_;  ///< Timer for checking client timeouts
  static constexpr std::chrono::seconds CLIENT_TIMEOUT{
      10};  ///< Client timeout duration
};

#ifdef _WIN32
  extern "C" {
    __declspec(dllexport) INetworkManager* EntryPointLib();
  }
#else
extern "C" {
INetworkManager* EntryPointLib();
}
#endif
