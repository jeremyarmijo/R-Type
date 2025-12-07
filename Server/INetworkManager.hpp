#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include "NetworkMessage.hpp"

class INetworkManager {
public:
    virtual ~INetworkManager() = default;

    // Callbacks
    using MessageCallback = std::function<void(const NetworkMessage&)>;
    using ConnectionCallback = std::function<void(uint32_t client_id)>;
    using DisconnectionCallback = std::function<void(uint32_t client_id)>;

    // Configuration
    virtual bool Initialize(uint16_t port) = 0;
    virtual void Shutdown() = 0;

    // Envoi/Réception
    virtual void SendTo(uint32_t client_id, const NetworkMessage& msg) = 0;
    virtual void Broadcast(const NetworkMessage& msg) = 0;
    virtual void Update() = 0; // Traiter les messages en attente

    // Callbacks
    virtual void SetMessageCallback(MessageCallback callback) = 0;
    virtual void SetConnectionCallback(ConnectionCallback callback) = 0;
    virtual void SetDisconnectionCallback(DisconnectionCallback callback) = 0;
};
