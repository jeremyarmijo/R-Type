#include "include/NetworkManager.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "include/DecodFunc.hpp"
#include "include/EncodeFunc.hpp"

NetworkManager::NetworkManager() : eventBuffer(50), actionBuffer(50) {
  SetupDecoder(decoder);
  SetupEncoder(encoder);
}

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

  int flags = fcntl(tcpSocket, F_GETFL, 0);
  if (flags < 0) flags = 0;
  fcntl(tcpSocket, F_SETFL, flags | O_NONBLOCK);

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

  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags < 0) flags = 0;
  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
    std::cerr << "Failed to set UDP socket non-blocking\n";
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
    if (evt.type == EventType::LOGIN_RESPONSE) {
      const auto* input = std::get_if<LOGIN_RESPONSE>(&evt.data);
      if (!input) return;
      udpPort = input->udpPort;
    }
    std::lock_guard<std::mutex> lock(mut);
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
    std::cout << "Serveur TCP déconnecté" << std::endl;
    tcpConnected = false;
  } else {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    } else {
      std::cerr << "TCP read error" << std::endl;
      tcpConnected = false;
    }
  }
}

void NetworkManager::SendACK(std::vector<uint8_t>& evt) {
  if (evt.size() < 6) return;

  uint8_t flag;
  memcpy(&flag, &evt[1], sizeof(flag));
  if (flag != 0x08) return;

  uint32_t sequenceNum;
  memcpy(&sequenceNum, &evt[6], sizeof(sequenceNum));

  std::vector<uint8_t> response(11);
  response[0] = 0x2A;
  response[1] = 0x02;
  uint32_t payloadSize = 5;
  memcpy(&response[2], &payloadSize, sizeof(uint32_t));

  memcpy(&response[6], &sequenceNum, sizeof(uint32_t));
  response[10] = evt[0];

  SendUdp(response);
}

void NetworkManager::ReadUDP() {
  char tempBuffer[2048];
  int bytesReceived = recv(udpSocket, tempBuffer, sizeof(tempBuffer), 0);

  if (bytesReceived > 0) {
    std::vector<uint8_t> packet(tempBuffer, tempBuffer + bytesReceived);
    SendACK(packet);
    Event evt = DecodePacket(packet);
    std::lock_guard<std::mutex> lock(mut);
    eventBuffer.push(evt);
  } else if (bytesReceived == 0) {
    std::cout << "UDP server disconnected" << std::endl;
    udpConnected = false;
  } else {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    } else {
      std::cerr << "UDP read error" << std::endl;
      udpConnected = false;
    }
  }
}

void NetworkManager::SendUdp(std::vector<uint8_t>& packet) {
  if (!udpConnected || udpSocket < 0) return;

  int sent = send(udpSocket, packet.data(), packet.size(), MSG_DONTWAIT);
  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      std::cerr << "UDP Send would block, buffer full\n";
      return;
    }
    std::cerr << "UDP Send error: " << strerror(errno) << std::endl;
    udpConnected = false;
    return;
  }
  std::cout << "UDP Port =" << udpPort << "\n";
  std::cout << "UDP message Send (" << sent << " bytes)\n";
  sequenceNumUdp++;
}

void NetworkManager::SendTcp(std::vector<uint8_t>& packet) {
  if (!tcpConnected || tcpSocket < 0) return;

  size_t totalSent = 0;
  while (totalSent < packet.size()) {
    int sent = send(tcpSocket, packet.data() + totalSent,
                    packet.size() - totalSent, MSG_DONTWAIT);

    if (sent < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::cerr << "TCP Send would block, buffer full. Sent " << totalSent
                  << "/" << packet.size() << " bytes\n";
        break;
      }
      std::cerr << "TCP Send error: " << strerror(errno) << std::endl;
      tcpConnected = false;
      return;
    }

    totalSent += sent;
  }

  if (totalSent == packet.size()) {
    sequenceNumTcp++;
  }
}

void NetworkManager::SendActionServer() {
  std::unique_lock<std::mutex> lock(mut);
  size_t bufferSize = actionBuffer.size();

  for (size_t i = 0; i < bufferSize; ++i) {
    auto opt = actionBuffer.peek();
    if (!opt.has_value()) break;

    const Action& action = opt.value();
    size_t protocol = UseUdp(action.type);

    bool sent = false;
    if ((protocol == 0 || protocol == 1) && udpConnected) {
      std::vector<uint8_t> packet =
          encoder.encode(action, protocol, sequenceNumUdp);

      lock.unlock();
      SendUdp(packet);
      lock.lock();

      sent = true;
    } else if (protocol == 2 && tcpConnected) {
      std::vector<uint8_t> packet =
          encoder.encode(action, protocol, sequenceNumTcp);

      lock.unlock();
      SendTcp(packet);
      lock.lock();

      sent = true;
    }

    if (sent) {
      actionBuffer.pop();
    } else {
      break;
    }
  }
}

void NetworkManager::ThreadLoop() {
  while (running) {
    if (!tcpConnected) {
      ConnectTCP();
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
    SendActionServer();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

Event NetworkManager::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  auto res = eventBuffer.pop();
  if (res.has_value()) return res.value();
  return Event{EventType::UNKNOWN};
}

void NetworkManager::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);
}
