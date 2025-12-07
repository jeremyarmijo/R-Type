#pragma once
#include "network/INetworkManager.hpp"
#include "network/UDPServer.hpp"
#include "network/ClientManager.hpp"
#include <asio.hpp>
#include <thread>
#include <queue>
#include <mutex>

// Forward declaration
class TCPServer;

// Structure pour les événements de connexion/déconnexion
struct ConnectionEvent {
    enum Type { CONNECTED, DISCONNECTED };
    Type type;
    uint32_t client_id;
};

class ServerNetworkManager : public INetworkManager {
public:
    ServerNetworkManager();
    ~ServerNetworkManager() override;

    bool Initialize(uint16_t tcp_port, uint16_t udp_port);
    bool Initialize(uint16_t port) override;  // Backward compatibility
    void Shutdown() override;
    
    void SendTo(uint32_t client_id, const NetworkMessage& msg) override;
    void Broadcast(const NetworkMessage& msg) override;
    void Update() override;
    
    void SetMessageCallback(MessageCallback callback) override;
    void SetConnectionCallback(ConnectionCallback callback) override;
    void SetDisconnectionCallback(DisconnectionCallback callback) override;

private:
    void IOThreadFunc();
    void CheckClientTimeouts();
    void OnReceive(const std::vector<uint8_t>& data, 
                   const asio::ip::udp::endpoint& sender);
    void OnTCPLogin(uint32_t client_id, const std::string& username, 
                    const asio::ip::tcp::endpoint& tcp_endpoint);
    void OnTCPDisconnect(uint32_t client_id);

    asio::io_context io_context_;
    std::unique_ptr<asio::io_context::work> work_guard_;
    std::unique_ptr<TCPServer> tcp_server_;
    std::unique_ptr<UDPServer> udp_server_;
    std::thread io_thread_;
    
    ClientManager client_manager_;
    
    // Thread-safe message queue
    std::queue<NetworkMessage> incoming_messages_;
    std::mutex queue_mutex_;
    
    // Thread-safe connection events queue
    std::queue<ConnectionEvent> connection_events_;
    std::mutex events_mutex_;
    
    // Callbacks
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    
    // Timeout timer
    std::unique_ptr<asio::steady_timer> timeout_timer_;
    static constexpr std::chrono::seconds CLIENT_TIMEOUT{10};
};
