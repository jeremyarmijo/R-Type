#pragma once

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "network/CircularBuffer.hpp"
#include "network/Decoder.hpp"
#include "network/Event.hpp"

class NetworkManager {
 public:
  NetworkManager();
  ~NetworkManager() {}

  bool Connect(const std::string& ip, int port);
  void Disconnect();

  bool IsConnected() const { return connected; }

  void ThreadLoop();
  void SendInput();
  Event PopEvent();

 private:
  bool connected = false;
  bool tcpConnected = false;
  bool udpConnected = false;

  bool running = false;

  int tcpSocket = -1;
  int tcpPort = -1;
  int udpPort = -1;
  int udpSocket = -1;
  std::string serverIP;

  std::thread networkThread;

  int ConnectTCP();
  int ConnectUDP();

  void ReadTCP();
  void ReadUDP();

  void ProcessTCPRecvBuffer();

  Decoder decoder;
  Event DecodePacket(std::vector<uint8_t>& packet);

  std::vector<uint8_t> recvTcpBuffer;
  CircularBuffer<Event> eventBuffer;
};
