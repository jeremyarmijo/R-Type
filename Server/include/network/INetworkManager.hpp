#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include "include/NetworkMessage.hpp"

/**
 * @class INetworkManager
 * @brief Interface for network management implementations
 *
 * Defines the contract for network managers handling client-server
 * communication.
 */
class INetworkManager {
 public:
  /**
   * @brief Virtual destructor
   */
  virtual ~INetworkManager() = default;

  using MessageCallback =
      std::function<void(const NetworkMessage&)>;  ///< Message callback type
  using ConnectionCallback =
      std::function<void(uint32_t client_id)>;  ///< Connection callback type
  using DisconnectionCallback =
      std::function<void(uint32_t client_id)>;  ///< Disconnection callback type

  virtual bool Initialize(uint16_t tcp_port, uint16_t udp_port,
                          const std::string& host) = 0;
  virtual void Shutdown() = 0;

  /**
   * @brief Send message to specific client
   * @param msg Network message to send
   * @param sendUdp If true, use UDP; otherwise use TCP
   */
  virtual void SendTo(const NetworkMessage& msg, bool sendUdp) = 0;

  /**
   * @brief Broadcast message via UDP to all clients
   * @param msg Network message to broadcast
   */
  virtual void BroadcastLobbyUDP(const NetworkMessage& msg,
                                 std::vector<std::tuple<uint16_t, bool, std::string>>&) = 0;

  /**
   * @brief Broadcast message via TCP to all clients
   * @param msg Network message to broadcast
   */
  virtual void BroadcastLobbyTCP(const NetworkMessage& msg,
                                 std::vector<std::tuple<uint16_t, bool, std::string>>&) = 0;

  /**
   * @brief Update network state and process events
   */
  virtual void Update() = 0;

  /**
   * @brief Set callback for incoming messages
   * @param callback Function to call when message is received
   */
  virtual void SetMessageCallback(MessageCallback callback) = 0;

  /**
   * @brief Set callback for client connections
   * @param callback Function to call when client connects
   */
  virtual void SetConnectionCallback(ConnectionCallback callback) = 0;

  /**
   * @brief Set callback for client disconnections
   * @param callback Function to call when client disconnects
   */
  virtual void SetDisconnectionCallback(DisconnectionCallback callback) = 0;
};
