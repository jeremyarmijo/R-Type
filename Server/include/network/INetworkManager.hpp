#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include "NetworkMessage.hpp"

class INetworkManager {
public:
    virtual ~INetworkManager() = default;

    using MessageCallback = std::function<void(const NetworkMessage&)>;
    using ConnectionCallback = std::function<void(uint32_t client_id)>;
    using DisconnectionCallback = std::function<void(uint32_t client_id)>;

    virtual bool Initialize(uint16_t tcp_port, uint16_t udp_port) = 0;
    virtual void Shutdown() = 0;

    virtual void SendTo(uint32_t client_id, const NetworkMessage& msg) = 0;
    virtual void Broadcast(const NetworkMessage& msg) = 0;
    virtual void Update() = 0;

    virtual void SetMessageCallback(MessageCallback callback) = 0;
    virtual void SetConnectionCallback(ConnectionCallback callback) = 0;
    virtual void SetDisconnectionCallback(DisconnectionCallback callback) = 0;
};
