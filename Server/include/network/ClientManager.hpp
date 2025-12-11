#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "network/HandleClient.hpp"

class ClientManager {
 public:
  using ClientPtr = std::shared_ptr<HandleClient>;

  ClientPtr AddClientFromTCP(const asio::ip::tcp::endpoint& tcp_endpoint,
                             const std::string& username, uint32_t assigned_id);

  bool AssociateUDPEndpoint(uint32_t client_id,
                            const asio::ip::udp::endpoint& udp_endpoint);

  void RemoveClient(uint32_t client_id);
  ClientPtr GetClient(uint32_t client_id);

  std::vector<ClientPtr> GetAllClients();
  size_t GetClientCount();

  std::vector<uint32_t> CheckTimeouts(std::chrono::seconds timeout);

 private:
  std::unordered_map<uint32_t, ClientPtr> clients_;
  std::mutex mutex_;
  uint32_t next_id_ = 1;
};
