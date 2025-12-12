#pragma once

#include <asio.hpp>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "include/Action.hpp"
#include "include/CircularBuffer.hpp"
#include "include/Decoder.hpp"
#include "include/Encode.hpp"
#include "include/Event.hpp"

class NetworkManager {
 public:
  NetworkManager();
  ~NetworkManager();

  bool Connect(const std::string& ip, int port);
  void Disconnect();

  bool IsConnected() const { return tcpConnected; }

  void SendAction(Action action);
  Event PopEvent();

 private:
  std::mutex mut;
  bool tcpConnected = false;
  bool udpConnected = false;

  bool running = false;

  uint16_t playerId = 0;

  int tcpPort = -1;
  int udpPort = -1;
  std::string serverIP;

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

  std::vector<uint8_t> recvTcpBuffer;
  CircularBuffer<Event> eventBuffer;

  CircularBuffer<Action> actionBuffer;

  Encoder encoder;
  // uint32_t sequenceNumUdp = 0;
  // uint32_t sequenceNumTcp = 0;
  void SendActionServer();
};
