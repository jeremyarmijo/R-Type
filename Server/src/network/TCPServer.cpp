#include "network/TCPServer.hpp"

#include <iostream>

TCPServer::TCPServer(asio::io_context& io_context, uint16_t port)
    : acceptor_(io_context,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), next_client_id_(1), udp_port_(PORT_UDP_DEFAULT) {
  std::cout << "[TCPServer] Listening on port " << port << std::endl;
  StartAccept();
}

TCPServer::~TCPServer() { acceptor_.close(); }

void TCPServer::SetLoginCallback(LoginCallback callback) {
  login_callback_ = std::move(callback);
}

void TCPServer::SetDisconnectCallback(DisconnectCallback callback) {
  disconnect_callback_ = std::move(callback);
}

void TCPServer::StartAccept() {
  acceptor_.async_accept(
      [this](const asio::error_code& error, asio::ip::tcp::socket socket) {
        if (!error) {
          uint32_t client_id = next_client_id_++;
          auto session = std::make_shared<ProcessPacketTCP>(std::move(socket),
                                                            client_id, this);
          {
            std::lock_guard<std::mutex> lock(process_packet_mutex_);
            sessions_[client_id] = session;
          }

          session->Start();
        }

        StartAccept();
      });
}

void TCPServer::DisconnectClient(uint32_t client_id) {
  std::lock_guard<std::mutex> lock(process_packet_mutex_);
  auto it = sessions_.find(client_id);
  if (it != sessions_.end()) {
    it->second->Close();
    sessions_.erase(it);
  }
}

ProcessPacketTCP::ProcessPacketTCP(asio::ip::tcp::socket socket,
                                   uint32_t client_id, TCPServer* server)
    : socket_(std::move(socket)),
      endpoint_(socket_.remote_endpoint()),
      client_id_(client_id),
      server_(server) {
  std::cout << "[ProcessPacketTCP] New connection from "
            << endpoint_.address().to_string() << ":" << endpoint_.port()
            << " (Client ID: " << client_id_ << ")" << std::endl;
}

ProcessPacketTCP::~ProcessPacketTCP() {
  std::cout << "[ProcessPacketTCP] Disconnected client " << client_id_
            << std::endl;
}

void ProcessPacketTCP::Start() { ReadHeader(); }

void ProcessPacketTCP::Close() {
  if (socket_.is_open()) {
    asio::error_code ec;
    socket_.close(ec);
  }
}

void ProcessPacketTCP::ReadHeader() {
  auto self = shared_from_this();
  asio::async_read(
      socket_, asio::buffer(header_buffer_),
      [this, self](const asio::error_code& error, std::size_t bytes) {
        HandleReadHeader(error, bytes);
      });
}

void ProcessPacketTCP::HandleReadHeader(const asio::error_code& error,
                                        std::size_t bytes_transferred) {
  if (error) {
    if (error != asio::error::eof && error != asio::error::operation_aborted) {
      std::cerr << "[ProcessPacketTCP] Header read error: " << error.message()
                << std::endl;
    }

    // Notifier la déconnexion
    if (server_->disconnect_callback_ && authenticated_) {
      server_->disconnect_callback_(client_id_);
    }
    return;
  }

  // Parser header: Type(1) + Flags(1) + Length(4 big-endian)
  uint8_t type = header_buffer_[0];
  uint8_t flags = header_buffer_[1];
  uint32_t length = (static_cast<uint32_t>(header_buffer_[2]) << 24) |
                    (static_cast<uint32_t>(header_buffer_[3]) << 16) |
                    (static_cast<uint32_t>(header_buffer_[4]) << 8) |
                    static_cast<uint32_t>(header_buffer_[5]);

  std::cout << "[ProcessPacketTCP] Received header: type=0x" << std::hex
            << (int)type << " flags=0x" << (int)flags << " length=" << std::dec
            << length << std::endl;

  // Vérifier taille raisonnable
  if (length > 1024) {
    std::cerr << "[ProcessPacketTCP] Payload too large: " << length
              << std::endl;
    Close();
    return;
  }

  // Stocker le type pour le traitement
  current_msg_type_ = type;

  // Lire le payload
  if (length > 0) {
    ReadPayload(length);
  } else {
    // Pas de payload, relire header
    ReadHeader();
  }
}

void ProcessPacketTCP::ReadPayload(uint16_t payload_size) {
  payload_buffer_.resize(payload_size);

  auto self = shared_from_this();
  asio::async_read(
      socket_, asio::buffer(payload_buffer_),
      [this, self](const asio::error_code& error, std::size_t bytes) {
        HandleReadPayload(error, bytes);
      });
}

void ProcessPacketTCP::HandleReadPayload(const asio::error_code& error,
                                         std::size_t bytes_transferred) {
  if (error) {
    std::cerr << "[ProcessPacketTCP] Payload read error: " << error.message()
              << std::endl;
    return;
  }

  // Traiter selon le type de message
  switch (current_msg_type_) {
    case 0x01:  // LOGIN_REQUEST
      ProcessLoginRequest();
      break;

    default:
      std::cerr << "[ProcessPacketTCP] Unknown message type: 0x" << std::hex
                << (int)current_msg_type_ << std::endl;
      break;
  }

  // Lire le prochain message
  ReadHeader();
}

void ProcessPacketTCP::ProcessLoginRequest() {
  if (payload_buffer_.size() < 2) {
    SendLoginError(0x1004, "Invalid login request");
    return;
  }

  size_t offset = 0;

  // Lire username
  uint8_t username_len = payload_buffer_[offset++];
  if (offset + username_len > payload_buffer_.size()) {
    SendLoginError(0x1004, "Invalid username length");
    return;
  }

  username_ = std::string(reinterpret_cast<char*>(&payload_buffer_[offset]),
                          username_len);
  offset += username_len;

  // Lire password (on ne le vérifie pas pour l'instant)
  if (offset >= payload_buffer_.size()) {
    SendLoginError(0x1004, "Missing password");
    return;
  }

  uint8_t password_len = payload_buffer_[offset++];
  if (offset + password_len > payload_buffer_.size()) {
    SendLoginError(0x1004, "Invalid password length");
    return;
  }

  std::string password(reinterpret_cast<char*>(&payload_buffer_[offset]),
                       password_len);

  // TODO: Vérifier credentials dans une base de données
  // Pour l'instant, on accepte tout le monde

  std::cout << "[ProcessPacketTCP] Login request from '" << username_
            << "' (client " << client_id_ << ")" << std::endl;

  authenticated_ = true;

  // Notifier le succès avec callback
  if (server_->login_callback_) {
    server_->login_callback_(client_id_, username_, endpoint_);
  }

  // Envoyer réponse
  SendLoginResponse(true, client_id_, server_->GetUDPPort());
}

void ProcessPacketTCP::SendLoginResponse(bool success, uint32_t player_id,
                                         uint16_t udp_port) {
  std::vector<uint8_t> response;

  // Header: Type(1) + Flags(1) + Length(4)
  response.push_back(0x02);  // Type: LOGIN_RESPONSE
  response.push_back(0x01);  // Flags: TCP protocol

  if (success) {
    // Length = 9 bytes (success + playerId + serverTick + udpPort)
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x09);

    // Payload
    response.push_back(0x01);                     // Success = 1
    response.push_back((player_id >> 8) & 0xFF);  // Player ID high byte
    response.push_back(player_id & 0xFF);         // Player ID low byte

    // Server tick (placeholder: 1)
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x01);

    // UDP port
    response.push_back((udp_port >> 8) & 0xFF);
    response.push_back(udp_port & 0xFF);
  } else {
    // Length = 1 byte (success = 0)
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x01);

    response.push_back(0x00);  // Success = 0
  }

  auto self = shared_from_this();
  asio::async_write(
      socket_, asio::buffer(response),
      [this, self](const asio::error_code& error, std::size_t bytes) {
        if (error) {
          std::cerr << "[ProcessPacketTCP] Send error: " << error.message()
                    << std::endl;
        } else {
          std::cout << "[ProcessPacketTCP] LOGIN_RESPONSE sent to client "
                    << client_id_ << std::endl;
        }
      });
}

void ProcessPacketTCP::SendLoginError(uint16_t error_code,
                                      const std::string& message) {
  std::vector<uint8_t> response;

  // Header: Type(1) + Flags(1) + Length(4)
  response.push_back(0x02);  // Type: LOGIN_RESPONSE
  response.push_back(0x01);  // Flags: TCP protocol

  // Length = 4 + message.size() (success + errorCode + messageLen + message)
  uint32_t payload_size = 4 + message.size();
  response.push_back((payload_size >> 24) & 0xFF);
  response.push_back((payload_size >> 16) & 0xFF);
  response.push_back((payload_size >> 8) & 0xFF);
  response.push_back(payload_size & 0xFF);

  // Payload (échec)
  response.push_back(0x00);                      // Success = 0
  response.push_back((error_code >> 8) & 0xFF);  // Error code high byte
  response.push_back(error_code & 0xFF);         // Error code low byte
  response.push_back(static_cast<uint8_t>(message.size()));

  for (char c : message) {
    response.push_back(static_cast<uint8_t>(c));
  }

  auto self = shared_from_this();
  asio::async_write(
      socket_, asio::buffer(response),
      [this, self](const asio::error_code& error, std::size_t bytes) {
        if (!error) {
          std::cout << "[ProcessPacketTCP] LOGIN_ERROR sent to client "
                    << client_id_ << std::endl;
        }
        Close();  // Fermer après erreur
      });
}
