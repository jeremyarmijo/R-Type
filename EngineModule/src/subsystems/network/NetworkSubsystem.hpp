#pragma once

#include <cstring>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>

#include "engine/ISubsystem.hpp"
#include "network/Action.hpp"
#include "network/CircularBuffer.hpp"
#include "network/Decoder.hpp"
#include "network/Encoder.hpp"
#include "network/Event.hpp"
#include "network/network_export.hpp"

class NETWORK_API NetworkSubsystem : public ISubsystem {
 public:
  struct SentPacket {
    uint16_t seq;
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point lastSent;
    int retryCount;
  };
  std::map<uint16_t, SentPacket> clientHistory;
  NetworkSubsystem();
  ~NetworkSubsystem() override;

  bool Initialize() override;
  void Shutdown() override;
  void Update(float deltaTime) override;
  void SetRegistry(Registry* registry) override {}
  void ProcessEvent(SDL_Event event) override {}
  const char* GetName() const override { return "Network"; }
  SubsystemType GetType() const override { return SubsystemType::NETWORK; }
  const char* GetVersion() const override { return "1.0.0"; }

  bool Connect(const std::string& ip, int port);
  void Disconnect();

  bool IsConnected() const { return tcpConnected; }

  void SendAction(Action action);
  Event PopEvent();
  /*bool HasMapData() const { return mapDataReceived; }
    uint16_t GetMapWidth() const { return mapWidth; }
    uint16_t GetMapHeight() const { return mapHeight; }
    float GetMapScrollSpeed() const { return mapScrollSpeed; }
    const std::vector<uint8_t>& GetMapTiles() const { return mapTiles; }
    void ClearMapData() { mapDataReceived = false; mapTiles.clear(); }*/
 private:
  std::mutex mut;
  bool tcpConnected = false;
  bool udpConnected = false;

  bool running = false;

  uint16_t playerId = 0;

  int tcpPort = -1;
  int udpPort = -1;
  std::string serverIP;

  uint16_t seqNum = 1;
  uint16_t ack = 0;
  uint32_t ack_bits = 0;

  asio::io_context ioContext;
  asio::ip::tcp::socket tcpSocket;
  asio::ip::udp::socket udpSocket;
  asio::ip::udp::endpoint udpEndpoint;

  std::thread networkThread;

  void ThreadLoop();

  int ConnectTCP();
  int ConnectUDP();

  void ReadTCP();
  void ReadUDP();

  void SendUdp(std::vector<uint8_t>& packet);
  // void SendACK(std::vector<uint8_t>& evt);
  void SendTcp(std::vector<uint8_t>& packet);

  void AuthAction();

  void ProcessTCPRecvBuffer();

  Decoder decoder;
  Event DecodePacket(std::vector<uint8_t>& packet);
  void HandleRetransmission();

  std::vector<uint8_t> recvTcpBuffer;
  CircularBuffer<Event> eventBuffer;

  CircularBuffer<Action> actionBuffer;

  Encoder encoder;
  // uint32_t sequenceNumUdp = 0;
  // uint32_t sequenceNumTcp = 0;
  void SendActionServer();
};
#ifdef _WIN32
extern "C" {
__declspec(dllexport) ISubsystem* CreateSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem();
}
#endif