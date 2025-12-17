#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "network/HandleClient.hpp"

/**
 * @class ClientManager
 * @brief Manages all connected clients and their state
 * 
 * Thread-safe manager for tracking clients, associating endpoints,
 * and checking for timeouts.
 */
class ClientManager {
 public:
  using ClientPtr = std::shared_ptr<HandleClient>; ///< Shared pointer to client

  /**
   * @brief Add a new client from TCP connection
   * @param tcp_endpoint Client's TCP endpoint
   * @param username Client's username
   * @param assigned_id Assigned client ID
   * @return Shared pointer to the new client
   */
  ClientPtr AddClientFromTCP(const asio::ip::tcp::endpoint& tcp_endpoint,
                             const std::string& username, uint16_t assigned_id);

  /**
   * @brief Associate UDP endpoint with existing client
   * @param client_id Client identifier
   * @param udp_endpoint UDP endpoint to associate
   * @return true if successful, false if client not found
   */
  bool AssociateUDPEndpoint(uint16_t client_id,
                            const asio::ip::udp::endpoint& udp_endpoint);

  /**
   * @brief Remove a client by ID
   * @param client_id Client identifier to remove
   */
  void RemoveClient(uint16_t client_id);
  
  /**
   * @brief Get client by ID
   * @param client_id Client identifier
   * @return Shared pointer to client, or nullptr if not found
   */
  ClientPtr GetClient(uint16_t client_id);
  
  /**
   * @brief Get client by UDP endpoint
   * @param ep UDP endpoint to search for
   * @return Shared pointer to client, or nullptr if not found
   */
  ClientPtr GetUDPClientByEndpoint(const asio::ip::udp::endpoint& ep);

  /**
   * @brief Get all connected clients
   * @return Vector of client pointers
   */
  std::vector<ClientPtr> GetAllClients();
  
  /**
   * @brief Get number of connected clients
   * @return Client count
   */
  size_t GetClientCount();

  /**
   * @brief Check for timed out clients
   * @param timeout Timeout duration
   * @return Vector of timed out client IDs
   */
  std::vector<uint32_t> CheckTimeouts(std::chrono::seconds timeout);

 private:
  std::unordered_map<uint16_t, ClientPtr> clients_; ///< Map of client ID to client object
  std::mutex mutex_;                                 ///< Mutex for thread-safe access
  uint16_t next_id_ = 1;                             ///< Next available client ID
};
