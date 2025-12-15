#include "network/ClientManager.hpp"

#include <memory>
#include <string>
#include <vector>

ClientManager::ClientPtr ClientManager::AddClientFromTCP(
    const asio::ip::tcp::endpoint& tcp_endpoint, const std::string& username,
    uint16_t assigned_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = clients_.find(assigned_id);
  if (it != clients_.end()) {
    return it->second;
  }

  auto client =
      std::make_shared<HandleClient>(assigned_id, tcp_endpoint, username);
  clients_[assigned_id] = client;

  if (assigned_id >= next_id_) {
    next_id_ = assigned_id + 1;
  }

  return client;
}

bool ClientManager::AssociateUDPEndpoint(
    uint16_t client_id, const asio::ip::udp::endpoint& udp_endpoint) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = clients_.find(client_id);
  if (it == clients_.end()) {
    return false;
  }

  it->second->SetUDPEndpoint(udp_endpoint);

  return true;
}

void ClientManager::RemoveClient(uint16_t client_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = clients_.find(client_id);
  if (it != clients_.end()) {
    clients_.erase(it);
  }
}

ClientManager::ClientPtr ClientManager::GetUDPClientByEndpoint(
    const asio::ip::udp::endpoint& ep) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& [id, client] : clients_) {
    if (client->HasUDPEndpoint() && client->GetUDPEndpoint() == ep) {
      return client;
    }
  }
  return nullptr;
}

ClientManager::ClientPtr ClientManager::GetClient(uint16_t client_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = clients_.find(client_id);
  return (it != clients_.end()) ? it->second : nullptr;
}

std::vector<ClientManager::ClientPtr> ClientManager::GetAllClients() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<ClientPtr> result;
  result.reserve(clients_.size());
  for (auto& [id, client] : clients_) {
    result.push_back(client);
  }
  return result;
}

size_t ClientManager::GetClientCount() {
  std::lock_guard<std::mutex> lock(mutex_);
  return clients_.size();
}

std::vector<uint32_t> ClientManager::CheckTimeouts(
    std::chrono::seconds timeout) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<uint32_t> timed_out;

  for (auto& [id, client] : clients_) {
    if (client->IsTimedOut(timeout)) {
      timed_out.push_back(id);
    }
  }

  return timed_out;
}
