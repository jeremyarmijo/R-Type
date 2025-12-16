#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include "include/NetworkMessage.hpp"

class INetworkManager {
 public:
  virtual ~INetworkManager() = default;

  using MessageCallback = std::function<void(const NetworkMessage&)>;
  using ConnectionCallback = std::function<void(uint32_t client_id)>;
  using DisconnectionCallback = std::function<void(uint32_t client_id)>;

  virtual bool Initialize(uint16_t tcp_port, uint16_t udp_port, const std::string& host) = 0;
  virtual void Shutdown() = 0;

  virtual void SendTo(const NetworkMessage& msg, bool sendUdp) = 0;
  virtual void BroadcastUDP(const NetworkMessage& msg) = 0;
  virtual void BroadcastTCP(const NetworkMessage& msg) = 0;
  virtual void Update() = 0;

  virtual void SetMessageCallback(MessageCallback callback) = 0;
  virtual void SetConnectionCallback(ConnectionCallback callback) = 0;
  virtual void SetDisconnectionCallback(DisconnectionCallback callback) = 0;
};
