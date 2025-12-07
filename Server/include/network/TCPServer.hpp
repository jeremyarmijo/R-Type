#pragma once
#include <asio.hpp>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

// Forward declaration
class TCPSession;

class TCPServer {
public:
    using LoginCallback = std::function<void(
        uint32_t client_id,
        const std::string& username,
        const asio::ip::tcp::endpoint& endpoint
    )>;
    using DisconnectCallback = std::function<void(uint32_t client_id)>;

    TCPServer(asio::io_context& io_context, uint16_t port);
    ~TCPServer();

    void SetLoginCallback(LoginCallback callback);
    void SetDisconnectCallback(DisconnectCallback callback);
    
    void DisconnectClient(uint32_t client_id);
    uint16_t GetUDPPort() const { return udp_port_; }
    void SetUDPPort(uint16_t port) { udp_port_ = port; }

private:
    friend class TCPSession;  // Allow TCPSession to access callbacks
    
    void StartAccept();
    void HandleAccept(std::shared_ptr<TCPSession> session, const asio::error_code& error);

    asio::ip::tcp::acceptor acceptor_;
    LoginCallback login_callback_;
    DisconnectCallback disconnect_callback_;
    
    std::unordered_map<uint32_t, std::shared_ptr<TCPSession>> sessions_;
    std::mutex sessions_mutex_;
    
    uint32_t next_client_id_ = 1;
    uint16_t udp_port_ = 4243;
};

// Session TCP pour un client individuel
class TCPSession : public std::enable_shared_from_this<TCPSession> {
public:
    TCPSession(asio::ip::tcp::socket socket, uint32_t client_id, TCPServer* server);
    ~TCPSession();

    void Start();
    void Close();
    
    uint32_t GetClientId() const { return client_id_; }
    const asio::ip::tcp::endpoint& GetEndpoint() const { return endpoint_; }

private:
    void ReadHeader();
    void HandleReadHeader(const asio::error_code& error, std::size_t bytes_transferred);
    void ReadPayload(uint16_t payload_size);
    void HandleReadPayload(const asio::error_code& error, std::size_t bytes_transferred);
    
    void ProcessLoginRequest();
    void SendLoginResponse(bool success, uint32_t player_id, uint16_t udp_port);
    void SendLoginError(uint16_t error_code, const std::string& message);

    asio::ip::tcp::socket socket_;
    asio::ip::tcp::endpoint endpoint_;
    uint32_t client_id_;
    TCPServer* server_;
    
    std::array<uint8_t, 6> header_buffer_;  // Type(1) + Flags(1) + Length(4)
    std::vector<uint8_t> payload_buffer_;
    uint8_t current_msg_type_ = 0;
    
    bool authenticated_ = false;
    std::string username_;
};