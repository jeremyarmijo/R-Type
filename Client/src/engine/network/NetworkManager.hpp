#pragma once
#include "network/CircularBuffer.hpp"
#include "network/Event.hpp"
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class NetworkManager {

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
  int ConnectUDP() {};

  void ReadTCP();
  void ReadUDP() {};

  std::vector<uint8_t> recvBuffer;
  void ProcessRecvBuffer();

  Event DecodePacket(std::vector<uint8_t> &packet);
  CircularBuffer<Event> eventBuffer;

public:
  NetworkManager::NetworkManager() : eventBuffer(10) {};
  ~NetworkManager();

  bool Connect(const std::string &ip, int port);
  void Disconnect();

  bool IsConnected() const { return connected; }

  void ThreadLoop();

  void SendInput();

  Event PopEvent();
};
