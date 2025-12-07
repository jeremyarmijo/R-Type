#include "network/ServerNetworkManager.hpp"
#include "network/TCPServer.hpp"
#include <iostream>

ServerNetworkManager::ServerNetworkManager() = default;

ServerNetworkManager::~ServerNetworkManager() {
    Shutdown();
}

bool ServerNetworkManager::Initialize(uint16_t tcp_port, uint16_t udp_port) {
    try {
        work_guard_ = std::make_unique<asio::io_context::work>(io_context_);
        
        // Créer TCP server
        tcp_server_ = std::make_unique<TCPServer>(io_context_, tcp_port);
        tcp_server_->SetUDPPort(udp_port);
        
        tcp_server_->SetLoginCallback([this](uint32_t client_id, 
                                              const std::string& username,
                                              const asio::ip::tcp::endpoint& endpoint) {
            OnTCPLogin(client_id, username, endpoint);
        });
        
        tcp_server_->SetDisconnectCallback([this](uint32_t client_id) {
            OnTCPDisconnect(client_id);
        });
        
        // Créer UDP server
        udp_server_ = std::make_unique<UDPServer>(io_context_, udp_port);
        
        udp_server_->SetReceiveCallback([this](const auto& data, const auto& endpoint) {
            OnReceive(data, endpoint);
        });
        
        // Timer pour vérifier les timeouts
        timeout_timer_ = std::make_unique<asio::steady_timer>(io_context_);
        CheckClientTimeouts();
        
        // Démarrer le thread réseau
        io_thread_ = std::thread([this]() { IOThreadFunc(); });
        
        std::cout << "[ServerNetworkManager] Initialized TCP:" << tcp_port 
                  << " UDP:" << udp_port << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ServerNetworkManager] Initialization failed: " 
                  << e.what() << std::endl;
        return false;
    }
}

bool ServerNetworkManager::Initialize(uint16_t port) {
    // Backward compatibility: UDP only
    return Initialize(port, port + 1);
}

void ServerNetworkManager::Shutdown() {
    if (work_guard_) {
        work_guard_.reset();
        io_context_.stop();
    }
    
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
    
    tcp_server_.reset();
    udp_server_.reset();
    std::cout << "[ServerNetworkManager] Shutdown complete" << std::endl;
}

void ServerNetworkManager::IOThreadFunc() {
    std::cout << "[ServerNetworkManager] IO thread started" << std::endl;
    io_context_.run();
    std::cout << "[ServerNetworkManager] IO thread stopped" << std::endl;
}

void ServerNetworkManager::OnReceive(const std::vector<uint8_t>& data, 
                                     const asio::ip::udp::endpoint& sender) {
    // Vérifier taille minimale (au moins header avec playerId)
    if (data.size() < 3) {
        std::cerr << "[ServerNetworkManager] Invalid UDP packet size" << std::endl;
        return;
    }
    
    // Parser playerId du paquet (bytes 1-2 après le type)
    uint16_t player_id = (data[1] << 8) | data[2];
    
    // Trouver le client par ID (doit être authentifié via TCP)
    auto client = client_manager_.GetClient(player_id);
    
    if (!client) {
        std::cerr << "[ServerNetworkManager] UDP packet from unauthenticated client (ID: " 
                  << player_id << ")" << std::endl;
        return;
    }
    
    // Associer l'endpoint UDP si première fois
    if (!client->HasUDPEndpoint()) {
        client_manager_.AssociateUDPEndpoint(player_id, sender);
        std::cout << "[ServerNetworkManager] UDP endpoint associated for client " 
                  << player_id << std::endl;
    }
    
    client->UpdateLastSeen();
    client->IncrementPacketsReceived();
    
    // Ajouter à la queue pour traitement dans le game thread
    NetworkMessage msg;
    msg.client_id = client->GetId();
    msg.data = data;
    msg.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        incoming_messages_.push(std::move(msg));
    }
}

void ServerNetworkManager::Update() {
    // 1. Traiter les messages réseau
    std::queue<NetworkMessage> messages;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        messages.swap(incoming_messages_);
    }

    while (!messages.empty()) {
        if (message_callback_) {
            message_callback_(messages.front());
        }
        messages.pop();
    }
    
    // 2. Traiter les événements de connexion/déconnexion
    std::queue<ConnectionEvent> events;
    {
        std::lock_guard<std::mutex> lock(events_mutex_);
        events.swap(connection_events_);
    }
    
    while (!events.empty()) {
        auto& event = events.front();
        
        if (event.type == ConnectionEvent::CONNECTED) {
            if (connection_callback_) {
                connection_callback_(event.client_id);
            }
        } else if (event.type == ConnectionEvent::DISCONNECTED) {
            if (disconnection_callback_) {
                disconnection_callback_(event.client_id);
            }
        }
        
        events.pop();
    }
}

void ServerNetworkManager::SendTo(uint32_t client_id, const NetworkMessage& msg) {
    auto client = client_manager_.GetClient(client_id);
    if (client && udp_server_ && client->HasUDPEndpoint()) {
        udp_server_->SendTo(msg.data, client->GetUDPEndpoint());
        client->IncrementPacketsSent();
    }
}

void ServerNetworkManager::Broadcast(const NetworkMessage& msg) {
    auto clients = client_manager_.GetAllClients();
    for (const auto& client : clients) {
        if (udp_server_ && client->HasUDPEndpoint()) {
            udp_server_->SendTo(msg.data, client->GetUDPEndpoint());
            client->IncrementPacketsSent();
        }
    }
}

void ServerNetworkManager::CheckClientTimeouts() {
    auto timed_out = client_manager_.CheckTimeouts(CLIENT_TIMEOUT);
    
    for (uint32_t client_id : timed_out) {
        std::cout << "[ServerNetworkManager] Client " << client_id 
                  << " timed out" << std::endl;
        
        // Fermer la session TCP
        if (tcp_server_) {
            tcp_server_->DisconnectClient(client_id);
        }
        
        // Queue l'événement de déconnexion
        ConnectionEvent event{ConnectionEvent::DISCONNECTED, client_id};
        {
            std::lock_guard<std::mutex> lock(events_mutex_);
            connection_events_.push(event);
        }
        
        client_manager_.RemoveClient(client_id);
    }
    
    // Reschedule
    timeout_timer_->expires_after(std::chrono::seconds(1));
    timeout_timer_->async_wait([this](const asio::error_code&) {
        CheckClientTimeouts();
    });
}

void ServerNetworkManager::SetMessageCallback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void ServerNetworkManager::SetConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

void ServerNetworkManager::SetDisconnectionCallback(DisconnectionCallback callback) {
    disconnection_callback_ = std::move(callback);
}

void ServerNetworkManager::OnTCPLogin(uint32_t client_id,
                                      const std::string& username,
                                      const asio::ip::tcp::endpoint& tcp_endpoint) {
    // Ajouter le client au ClientManager
    auto client = client_manager_.AddClientFromTCP(tcp_endpoint, username, client_id);
    
    if (client) {
        std::cout << "[ServerNetworkManager] Client " << client_id 
                  << " (" << username << ") logged in via TCP" << std::endl;
        
        // Queue l'événement de connexion pour le game thread
        ConnectionEvent event{ConnectionEvent::CONNECTED, client_id};
        {
            std::lock_guard<std::mutex> lock(events_mutex_);
            connection_events_.push(event);
        }
    }
}

void ServerNetworkManager::OnTCPDisconnect(uint32_t client_id) {
    std::cout << "[ServerNetworkManager] Client " << client_id 
              << " disconnected from TCP" << std::endl;
    
    // Queue l'événement de déconnexion
    ConnectionEvent event{ConnectionEvent::DISCONNECTED, client_id};
    {
        std::lock_guard<std::mutex> lock(events_mutex_);
        connection_events_.push(event);
    }
    
    client_manager_.RemoveClient(client_id);
}
