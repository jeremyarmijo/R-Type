#pragma once
#include "./NetworkManager.hpp"
#include "./Decoder.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

bool NetworkManager::Connect(const std::string &ip, int port) {
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
    std::cerr << "Socket creation error\n";
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

  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Connexion server TCP error\n";
    close(sockfd);
    return -1;
  }

  tcpSocket = sockfd;
  tcpConnected = true;
  std::cout << "Connected to TCP server!\n";
  return 0;
}

Event NetworkManager::DecodePacket(std::vector<uint8_t> &packet) {
  Event newEvent;
  Decoder decode;
  newEvent.type = packet[0];
  uint8_t flag = packet[1];

  uint32_t lengthPayload;
  memcpy(&lengthPayload, &packet[2], sizeof(lengthPayload));

  const uint8_t *payloadPtr = packet.data() + 6;

  newEvent = decode.decode(packet);
  return newEvent;
}

void NetworkManager::ProcessRecvBuffer() {
  while (recvBuffer.size() >= 6) {

    uint32_t packetSize;
    memcpy(&packetSize, &recvBuffer[2], 4);

    if (recvBuffer.size() < 6 + packetSize)
      return;

    std::vector<uint8_t> packet;
    int end = 6 + packetSize;

    std::vector<uint8_t> packet(recvBuffer.begin(),
                                recvBuffer.begin() + 6 + packetSize);

    recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + 6 + packetSize);
    Event evt = DecodePacket(packet);
    eventBuffer.push(evt);
  }
}

void NetworkManager::ReadTCP() {
  char tempBuffer[1024];
  int bytesReceived = recv(tcpSocket, tempBuffer, sizeof(tempBuffer), 0);

  if (bytesReceived > 0) {
    recvBuffer.insert(recvBuffer.end(), tempBuffer, tempBuffer + bytesReceived);
    ProcessRecvBuffer();
  } else if (bytesReceived == 0) {
    std::cout << "Serveur déconnecté" << std::endl;
    tcpConnected = false;
  } else {
    std::cerr << "Erreur de lecture" << std::endl;
  }
}

void NetworkManager::ThreadLoop() {
  while (running) {

    if (!tcpConnected)
      if (ConnectTCP() == -1)
        return;

    if (udpPort != -1)
      ConnectUDP();

    if (tcpConnected)
      ReadTCP();
    if (udpConnected)
      ReadUDP();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
