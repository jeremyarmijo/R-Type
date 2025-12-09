#pragma once

#include <cstring>

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "Action.hpp"
#include "CircularBuffer.hpp"
#include "Decoder.hpp"
#include "Encode.hpp"
#include "Event.hpp"

class NetworkManager {
 public:
  NetworkManager();
  ~NetworkManager();

  bool Connect(const std::string& ip, int port);
  void Disconnect();

  bool IsConnected() const { return connected; }

  void SendAction(Action action);
  Event PopEvent();

 private:
  std::mutex mut;
  bool connected = false;
  bool tcpConnected = false;
  bool udpConnected = false;

  bool running = false;

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
  void SendACK(std::vector<uint8_t>& evt);
  void SendTcp(std::vector<uint8_t>& packet);

  void ProcessTCPRecvBuffer();

  Decoder decoder;
  Event DecodePacket(std::vector<uint8_t>& packet);

  std::vector<uint8_t> recvTcpBuffer;
  CircularBuffer<Event> eventBuffer;

  CircularBuffer<Action> actionBuffer;

  Encoder encoder;
  uint32_t sequenceNumUdp = 0;
  uint32_t sequenceNumTcp = 0;
  void SendActionServer();
};
