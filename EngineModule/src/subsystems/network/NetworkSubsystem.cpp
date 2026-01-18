#include "network/NetworkSubsystem.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"

NetworkSubsystem::NetworkSubsystem()
    : tcpSocket(ioContext),
      udpSocket(ioContext),
      eventBuffer(50),
      actionBuffer(50) {}

NetworkSubsystem::~NetworkSubsystem() { Shutdown(); }

bool NetworkSubsystem::Initialize() {
  SetupDecoder(decoder);
  SetupEncoder(encoder);
  return true;
}

void NetworkSubsystem::Shutdown() {
  std::cout << "Shutting down Network Subsystem..." << std::endl;
  Disconnect();
  if (networkThread.joinable()) {
    networkThread.join();
  }
}

void NetworkSubsystem::Update(float deltaTime) {}

bool NetworkSubsystem::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  if (running) {
    std::cerr << "Try connect to TCP Server with IP(" << serverIP << ")"
              << std::endl;
    return false;
  }
  running = true;
  networkThread = std::thread(&NetworkSubsystem::ThreadLoop, this);
  return true;
}

void NetworkSubsystem::Disconnect() {
  Action disconnect;
  ClientLeave l;
  disconnect.type = ActionType::CLIENT_LEAVE;
  l.playerId = 0;
  disconnect.data = l;

  std::vector<uint8_t> packet = encoder.encode(disconnect, 2, 0, 0, 0);
  SendTcp(packet);

  running = false;

  asio::error_code ec;
  if (tcpSocket.is_open()) tcpSocket.close(ec);
  if (udpSocket.is_open()) udpSocket.close(ec);

  tcpConnected = false;
  udpConnected = false;
  tcpPort = -1;
  udpPort = -1;
  if (networkThread.joinable()) {
    networkThread.join();
  }
}

int NetworkSubsystem::ConnectTCP() {
  try {
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(serverIP), tcpPort);

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

int NetworkSubsystem::ConnectUDP() {
  try {
    udpEndpoint =
        asio::ip::udp::endpoint(asio::ip::make_address(serverIP), udpPort);

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

Event NetworkSubsystem::DecodePacket(std::vector<uint8_t>& packet) {
  return decoder.decode(packet);
}

void NetworkSubsystem::AuthAction() {
  Action ac;
  AuthUDP authData;

  ac.type = ActionType::AUTH;
  authData.playerId = playerId;
  ac.data = authData;
  SendAction(ac);
  std::cout << "AUTH UDP (playerId = " << playerId << ")\n";
}

void NetworkSubsystem::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    std::cout << "[TCP DEBUG] Packet type: 0x" << std::hex
              << static_cast<int>(recvTcpBuffer[0]) << std::dec
              << " buffer size: " << recvTcpBuffer.size() << std::endl;

    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));
    packetSize = ntohl(packetSize);

    std::cout << "[TCP DEBUG] Payload size: " << packetSize << std::endl;

    if (recvTcpBuffer.size() < 6 + packetSize) {
      std::cout << "[TCP DEBUG] Incomplete packet, waiting..." << std::endl;
      return;
    }

    std::vector<uint8_t> packet(recvTcpBuffer.begin(),
                                recvTcpBuffer.begin() + 6 + packetSize);

    recvTcpBuffer.erase(recvTcpBuffer.begin(),
                        recvTcpBuffer.begin() + 6 + packetSize);

    Event evt = DecodePacket(packet);

    std::cout << "[TCP DEBUG] Decoded event type: "
              << static_cast<int>(evt.type) << std::endl;

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

void NetworkSubsystem::ReadTCP() {
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

/*void NetworkSubsystem::SendACK(std::vector<uint8_t>& evt) {
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

void NetworkSubsystem::ReadUDP() {
  char tempBuffer[2048];

  asio::error_code error;
  size_t bytesReceived =
      udpSocket.receive(asio::buffer(tempBuffer, sizeof(tempBuffer)), 0, error);

  if (!error && bytesReceived > 0) {
    std::vector<uint8_t> packet(tempBuffer, tempBuffer + bytesReceived);
    Event evt = DecodePacket(packet);
    std::lock_guard<std::mutex> lock(mut);
    uint16_t newSeq = evt.seqNum;
    bool isDuplicate = false;

    if (newSeq == ack) {
      isDuplicate = true;
    } else if (static_cast<uint16_t>(newSeq - ack) < 32768) {
      uint16_t shift = newSeq - ack;
      if (shift < 32) {
        ack_bits <<= shift;
        ack_bits |= (1 << (shift - 1));
      } else {
        ack_bits = 0;
      }
      ack = newSeq;
    } else {
      uint16_t shift = ack - newSeq;
      if (shift > 0 && shift <= 32) {
        if (ack_bits & (1 << (shift - 1))) {
          isDuplicate = true;
        }
        ack_bits |= (1 << (shift - 1));
      } else {
        isDuplicate = true;
      }
    }
    clientHistory.erase(evt.ack);
    for (int i = 0; i < 32; ++i) {
      if (evt.ack_bits & (1 << i)) {
        clientHistory.erase(static_cast<uint16_t>(evt.ack - (i + 1)));
      }
    }
    if (!isDuplicate) eventBuffer.push(evt);
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

void NetworkSubsystem::SendUdp(std::vector<uint8_t>& packet) {
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
}

void NetworkSubsystem::SendTcp(std::vector<uint8_t>& packet) {
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

void NetworkSubsystem::SendActionServer() {
  while (true) {
    Action action;
    bool hasAction = false;

    {
      std::lock_guard<std::mutex> lock(mut);
      if (auto opt = actionBuffer.peek()) {
        action = opt.value();
        hasAction = true;
      }
    }

    if (!hasAction) break;

    size_t protocol = UseUdp(action.type);
    bool sent = false;
    if ((protocol == 0 || protocol == 1) && udpConnected) {
      uint16_t s, a;
      uint32_t ab;
      {
        std::lock_guard<std::mutex> lock(mut);
        s = ++seqNum;
        a = ack;
        ab = ack_bits;
      }
      std::vector<uint8_t> packet = encoder.encode(action, protocol, s, a, ab);
      if (protocol == 1) {
        std::lock_guard<std::mutex> lock(mut);
        clientHistory[s] = {s, packet, std::chrono::steady_clock::now(), 0};
      }
      SendUdp(packet);
      sent = true;
    } else if (protocol == 2 && tcpConnected) {
      std::vector<uint8_t> packet = encoder.encode(action, protocol, 0, 0, 0);
      SendTcp(packet);
      sent = true;
    }

    if (sent) {
      std::lock_guard<std::mutex> lock(mut);
      actionBuffer.pop();
    } else {
      break;
    }
  }
}

void NetworkSubsystem::HandleRetransmission() {
  std::lock_guard<std::mutex> lock(mut);
  auto now = std::chrono::steady_clock::now();

  for (auto it = clientHistory.begin(); it != clientHistory.end();) {
    if (now - it->second.lastSent > std::chrono::milliseconds(100)) {
      if (it->second.retryCount < 15) {
        SendUdp(it->second.data);
        it->second.lastSent = now;
        it->second.retryCount++;
        ++it;
      } else {
        it = clientHistory.erase(it);
      }
    } else {
      ++it;
    }
  }
}

void NetworkSubsystem::ThreadLoop() {
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
      HandleRetransmission();
    }
    SendActionServer();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

Event NetworkSubsystem::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  auto res = eventBuffer.pop();
  if (res.has_value()) return res.value();
  return Event{EventType::UNKNOWN};
}

void NetworkSubsystem::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);
}
#ifdef _WIN32
__declspec(dllexport) ISubsystem* CreateSubsystem() {
    return new NetworkSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem() { return new NetworkSubsystem(); }
}
#endif