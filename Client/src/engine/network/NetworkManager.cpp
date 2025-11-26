#pragma once

#include "network/NetworkManager.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "network/DecodFunc.hpp"

NetworkManager::NetworkManager() : eventBuffer(50) { SetupDecoder(decoder); }

bool NetworkManager::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  running = true;
  networkThread = std::thread(&NetworkManager::ThreadLoop, this);
  return true;
}

void NetworkManager::Disconnect() { running = false; }

int NetworkManager::ConnectTCP() {
  int sockfd;
  struct sockaddr_in serverAddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "TCP Socket creation error\n";
    return -1;
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(tcpPort);

  if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Invalid address\n";
    close(sockfd);
    return -1;
  }

  if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    std::cerr << "Connexion server TCP error\n";
    close(sockfd);
    return -1;
  }

  tcpSocket = sockfd;
  tcpConnected = true;
  std::cout << "Connected to TCP server!\n";
  return 0;
}

int NetworkManager::ConnectUDP() {
  int sockfd;
  struct sockaddr_in serverAddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "UDP Socket creation error\n";
    return -1;
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(udpPort);

  if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Invalid address\n";
    close(sockfd);
    return -1;
  }

  if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    std::cerr << "Connexion server UDP error\n";
    close(sockfd);
    return -1;
  }

  udpSocket = sockfd;
  udpConnected = true;
  std::cout << "Connected to UDP server!\n";
  return 0;
}

Event NetworkManager::DecodePacket(std::vector<uint8_t>& packet) {
  return decoder.decode(packet);
}

void NetworkManager::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));

    if (recvTcpBuffer.size() < 6 + packetSize) {
      return;
    }

    std::vector<uint8_t> packet(recvTcpBuffer.begin(),
                                recvTcpBuffer.begin() + 6 + packetSize);

    recvTcpBuffer.erase(recvTcpBuffer.begin(),
                        recvTcpBuffer.begin() + 6 + packetSize);

    Event evt = DecodePacket(packet);
    eventBuffer.push(evt);
  }
}

void NetworkManager::ReadTCP() {
  char tempBuffer[1024];
  int bytesReceived = recv(tcpSocket, tempBuffer, sizeof(tempBuffer), 0);

  if (bytesReceived > 0) {
    recvTcpBuffer.insert(recvTcpBuffer.end(), tempBuffer,
                         tempBuffer + bytesReceived);
    ProcessTCPRecvBuffer();
  } else if (bytesReceived == 0) {
    std::cout << "Serveur déconnecté" << std::endl;
    tcpConnected = false;
  } else {
    std::cerr << "Erreur de lecture" << std::endl;
  }
}

void NetworkManager::ReadUDP() {
  char tempBuffer[2048];
  int bytesReceived = recv(udpSocket, tempBuffer, sizeof(tempBuffer), 0);

  if (bytesReceived > 0) {
    std::vector<uint8_t> packet(tempBuffer, tempBuffer + bytesReceived);
    // handelACK();
    Event evt = DecodePacket(packet);
    eventBuffer.push(evt);
  } else if (bytesReceived == 0) {
    std::cout << "UDP server disconnected" << std::endl;
    udpConnected = false;
  } else {
    std::cerr << "UDP read error" << std::endl;
  }
}

void NetworkManager::ThreadLoop() {
  while (running) {
    if (!tcpConnected) {
      if (ConnectTCP() == -1) {
        return;
      }
    }

    if (!udpConnected && udpPort != -1) {
      ConnectUDP();
    }

    if (tcpConnected) {
      ReadTCP();
    }

    if (udpConnected && udpPort != -1) {
      ReadUDP();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
