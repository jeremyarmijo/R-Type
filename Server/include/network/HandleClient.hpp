#pragma once
#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <string>
#include <optional>

class HandleClient {
public:
    HandleClient(uint32_t id, const asio::ip::tcp::endpoint& tcp_endpoint, const std::string& username);

    uint32_t GetId() const { return id_; }
    const std::string& GetUsername() const { return username_; }
    const asio::ip::tcp::endpoint& GetTCPEndpoint() const { return tcp_endpoint_; }
    
    // UDP endpoint management
    bool HasUDPEndpoint() const { return udp_endpoint_.has_value(); }
    const asio::ip::udp::endpoint& GetUDPEndpoint() const { return udp_endpoint_.value(); }
    void SetUDPEndpoint(const asio::ip::udp::endpoint& endpoint) { udp_endpoint_ = endpoint; }
    
    // Authentication
    bool IsAuthenticated() const { return is_authenticated_; }
    void SetAuthenticated(bool auth) { is_authenticated_ = auth; }
    
    // Heartbeat management
    void UpdateLastSeen();
    bool IsTimedOut(std::chrono::seconds timeout) const;
    
    // Statistics
    uint64_t GetPacketsSent() const { return packets_sent_; }
    uint64_t GetPacketsReceived() const { return packets_received_; }
    void IncrementPacketsSent() { ++packets_sent_; }
    void IncrementPacketsReceived() { ++packets_received_; }

private:
    uint32_t id_;
    std::string username_;
    asio::ip::tcp::endpoint tcp_endpoint_;
    std::optional<asio::ip::udp::endpoint> udp_endpoint_;
    bool is_authenticated_;
    std::chrono::steady_clock::time_point last_seen_;
    uint64_t packets_sent_ = 0;
    uint64_t packets_received_ = 0;
};