#include "network/ClientManager.hpp"
#include <sstream>

std::string ClientManager::EndpointToString(const asio::ip::udp::endpoint& ep) {
    std::ostringstream oss;
    oss << ep.address().to_string() << ":" << ep.port();
    return oss.str();
}

ClientManager::ClientPtr ClientManager::AddClientFromTCP(
    const asio::ip::tcp::endpoint& tcp_endpoint,
    const std::string& username,
    uint32_t assigned_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Vérifier si l'ID existe déjà
    auto it = clients_.find(assigned_id);
    if (it != clients_.end()) {
        return it->second;
    }
    
    // Créer nouveau client avec TCP endpoint
    auto client = std::make_shared<HandleClient>(assigned_id, tcp_endpoint, username);
    clients_[assigned_id] = client;
    
    // Mettre à jour next_id_ si nécessaire
    if (assigned_id >= next_id_) {
        next_id_ = assigned_id + 1;
    }
    
    return client;
}

bool ClientManager::AssociateUDPEndpoint(
    uint32_t client_id,
    const asio::ip::udp::endpoint& udp_endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        return false;
    }
    
    it->second->SetUDPEndpoint(udp_endpoint);
    
    // Ajouter à la map endpoint_to_id pour recherche rapide
    std::string ep_str = EndpointToString(udp_endpoint);
    endpoint_to_id_[ep_str] = client_id;
    
    return true;
}

void ClientManager::RemoveClient(uint32_t client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        // Supprimer l'entrée UDP endpoint si elle existe
        if (it->second->HasUDPEndpoint()) {
            std::string ep_str = EndpointToString(it->second->GetUDPEndpoint());
            endpoint_to_id_.erase(ep_str);
        }
        clients_.erase(it);
    }
}

ClientManager::ClientPtr ClientManager::GetClient(uint32_t client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(client_id);
    return (it != clients_.end()) ? it->second : nullptr;
}

ClientManager::ClientPtr ClientManager::FindClientByUDPEndpoint(
    const asio::ip::udp::endpoint& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string ep_str = EndpointToString(endpoint);
    auto it = endpoint_to_id_.find(ep_str);
    if (it != endpoint_to_id_.end()) {
        return clients_[it->second];
    }
    return nullptr;
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

size_t ClientManager::GetClientCount() const {
    // std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size();
}

std::vector<uint32_t> ClientManager::CheckTimeouts(std::chrono::seconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint32_t> timed_out;
    
    for (auto& [id, client] : clients_) {
        if (client->IsTimedOut(timeout)) {
            timed_out.push_back(id);
        }
    }
    
    return timed_out;
}
