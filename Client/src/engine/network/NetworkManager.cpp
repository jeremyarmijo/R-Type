#include "include/NetworkManager.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"

NetworkManager::NetworkManager()
    : eventBuffer(50),
      actionBuffer(50),
      tcpSocket(ioContext),
      udpSocket(ioContext) {
  SetupDecoder(decoder);
  SetupEncoder(encoder);
}

NetworkManager::~NetworkManager() {
  Disconnect();
  if (networkThread.joinable()) {
    networkThread.join();
  }
}

bool NetworkManager::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  if (running) {
    std::cerr << "Try connect to TCP Server with IP("
      << serverIP << ")" << std::endl;
    return false;
  }
  running = true;
  networkThread = std::thread(&NetworkManager::ThreadLoop, this);
  return true;
}

void NetworkManager::Disconnect() {
  running = false;

  asio::error_code ec;
  if (tcpSocket.is_open())
    tcpSocket.close(ec);
  if (udpSocket.is_open())
    udpSocket.close(ec);

  tcpConnected = false;
  udpConnected = false;
}

int NetworkManager::ConnectTCP() {
  try {
    asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(serverIP),
                                     tcpPort);

    asio::error_code error;
    tcpSocket.connect(endpoint, error);

    if (error) {
      std::cerr << "Connexion server TCP error: " << error.message() << "\n";
      return -1;
    }

    tcpSocket.non_blocking(true, error);
    if (error) {
      std::cerr << "Failed to set TCP socket non-blocking: " << error.message()
                << "\n";
      tcpSocket.close();
      return -1;
    }

    tcpConnected = true;
    std::cout << "Connected to TCP server!\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TCP connection exception: " << error.what() << "\n";
    return -1;
  }
}

int NetworkManager::ConnectUDP() {
  try {
    udpEndpoint = asio::ip::udp::endpoint(
        asio::ip::address::from_string(serverIP), udpPort);

    asio::error_code error;
    udpSocket.open(asio::ip::udp::v4(), error);

    if (error) {
      std::cerr << "UDP Socket creation error: " << error.message() << "\n";
      return -1;
    }

    udpSocket.connect(udpEndpoint, error);
    if (error) {
      std::cerr << "Connexion server UDP error: " << error.message() << "\n";
      udpSocket.close();
      return -1;
    }

    udpSocket.non_blocking(true, error);
    if (error) {
      std::cerr << "Failed to set UDP socket non-blocking: " << error.message()
                << "\n";
      udpSocket.close();
      return -1;
    }

    udpConnected = true;
    std::cout << "Connected to UDP server!\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "UDP connection exception: " << error.what() << "\n";
    return -1;
  }
}

Event NetworkManager::DecodePacket(std::vector<uint8_t>& packet) {
  return decoder.decode(packet);
}

void NetworkManager::AuthAction() {
  Action ac;
  AuthUDP authData;

  ac.type = ActionType::AUTH;
  authData.playerId = playerId;
  ac.data = authData;
  SendAction(ac);
  std::cout << "AUTH UDP (playerId = " << playerId << ")\n";
}

void NetworkManager::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));
    packetSize = ntohl(packetSize);

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
      if (input->success == 1) {
        udpPort = input->udpPort;
        std::cout << "UDP Port =" << udpPort << "\n";
        playerId = input->playerId;
        AuthAction();
      }
    }
    std::lock_guard<std::mutex> lock(mut);
    eventBuffer.push(evt);
  }
}

void NetworkManager::ReadTCP() {
  char tempBuffer[1024];

  asio::error_code error;
  size_t bytesReceived =
      tcpSocket.read_some(asio::buffer(tempBuffer, sizeof(tempBuffer)), error);

  if (!error && bytesReceived > 0) {
    recvTcpBuffer.insert(recvTcpBuffer.end(), tempBuffer,
                         tempBuffer + bytesReceived);
    ProcessTCPRecvBuffer();
  } else if (error == asio::error::eof) {
    std::cout << "Serveur TCP déconnecté" << std::endl;
    tcpConnected = false;
  } else if (error == asio::error::would_block ||
             error == asio::error::try_again) {
    return;
  } else if (error) {
    std::cerr << "TCP read error: " << error.message() << std::endl;
    tcpConnected = false;
  }
}

/*void NetworkManager::SendACK(std::vector<uint8_t>& evt) {
  if (evt.size() < 6) return;

  uint8_t flag;
  memcpy(&flag, &evt[1], sizeof(flag));
  if (flag != 0x08) return;

  uint32_t sequenceNum;
  memcpy(&sequenceNum, &evt[6], sizeof(sequenceNum));
  sequenceNum = ntohl(sequenceNum);

  std::vector<uint8_t> response(11);
  response[0] = 0x2A;
  response[1] = 0x02;

  uint32_t payloadSize = htonl(5);
  memcpy(&response[2], &payloadSize, sizeof(uint32_t));

  uint32_t seqNet = htonl(sequenceNum);
  memcpy(&response[6], &seqNet, sizeof(uint32_t));

  response[10] = evt[0];

  SendUdp(response);
}*/

void NetworkManager::ReadUDP() {
  char tempBuffer[2048];

  asio::error_code error;
  size_t bytesReceived =
      udpSocket.receive(asio::buffer(tempBuffer, sizeof(tempBuffer)), 0, error);

  if (!error && bytesReceived > 0) {
    std::vector<uint8_t> packet(tempBuffer, tempBuffer + bytesReceived);
    // SendACK(packet);
    Event evt = DecodePacket(packet);
    std::lock_guard<std::mutex> lock(mut);
    eventBuffer.push(evt);
  } else if (error == asio::error::eof) {
    std::cout << "UDP server disconnected" << std::endl;
    udpConnected = false;
  } else if (error == asio::error::would_block ||
             error == asio::error::try_again) {
    return;
  } else if (error) {
    std::cerr << "UDP read error: " << error.message() << std::endl;
    udpConnected = false;
  }
}

void NetworkManager::SendUdp(std::vector<uint8_t>& packet) {
  if (!udpConnected || !udpSocket.is_open()) return;

  asio::error_code error;
  size_t sent = udpSocket.send(asio::buffer(packet), 0, error);

  if (error) {
    if (error == asio::error::would_block || error == asio::error::try_again) {
      std::cerr << "UDP Send would block, buffer full\n";
      return;
    }
    std::cerr << "UDP Send error: " << error.message() << std::endl;
    udpConnected = false;
    return;
  }
  std::cout << "UDP message Send (" << sent << " bytes)\n";
  // sequenceNumUdp++;
}

void NetworkManager::SendTcp(std::vector<uint8_t>& packet) {
  if (!tcpConnected || !tcpSocket.is_open()) return;

  size_t totalSent = 0;
  while (totalSent < packet.size()) {
    asio::error_code error;
    size_t sent = tcpSocket.write_some(
        asio::buffer(packet.data() + totalSent, packet.size() - totalSent),
        error);

    if (error) {
      if (error == asio::error::would_block ||
          error == asio::error::try_again) {
        std::cerr << "TCP Send would block, buffer full. Sent " << totalSent
                  << "/" << packet.size() << " bytes\n";
        break;
      }
      std::cerr << "TCP Send error: " << error.message() << std::endl;
      tcpConnected = false;
      return;
    }

    totalSent += sent;
  }

  /*if (totalSent == packet.size()) {
    sequenceNumTcp++;
  }*/
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
      std::vector<uint8_t> packet = encoder.encode(action, protocol);

      lock.unlock();
      SendUdp(packet);
      lock.lock();

      sent = true;
    } else if (protocol == 2 && tcpConnected) {
      std::vector<uint8_t> packet = encoder.encode(action, protocol);

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
      if (ConnectTCP() == -1) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[TCP] try to reconnect in 10 seconds\n";
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
