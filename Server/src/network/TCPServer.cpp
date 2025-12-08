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

    if (server_->disconnect_callback_ && authenticated_) {
      server_->disconnect_callback_(client_id_);
    }
    return;
  }

  uint8_t type = header_buffer_[0];
  uint8_t flags = header_buffer_[1];
  uint32_t length = (static_cast<uint32_t>(header_buffer_[2]) << 24) |
                    (static_cast<uint32_t>(header_buffer_[3]) << 16) |
                    (static_cast<uint32_t>(header_buffer_[4]) << 8) |
                    static_cast<uint32_t>(header_buffer_[5]);

  std::cout << "[ProcessPacketTCP] Received header: type=0x" << std::hex
            << (int)type << " flags=0x" << (int)flags << " length=" << std::dec
            << length << std::endl;

  current_msg_type_ = type;

  if (length > 0) {
    ReadPayload(length);
  } else {
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

  switch (current_msg_type_) {
    case 0x01:
      ProcessLoginRequest();
      break;

    default:
      std::cerr << "[ProcessPacketTCP] Unknown message type: 0x" << std::hex
                << (int)current_msg_type_ << std::endl;
      break;
  }

  ReadHeader();
}

void ProcessPacketTCP::ProcessLoginRequest() {
  size_t offset = 0;

  uint8_t username_len = payload_buffer_[offset++];
  if (offset + username_len > payload_buffer_.size()) {
    SendLoginError(0x1004, "Invalid username length");
    return;
  }

  username_ = std::string(reinterpret_cast<char*>(&payload_buffer_[offset]),
                          username_len);
  offset += username_len;

  std::cout << "[ProcessPacketTCP] Login request from '" << username_
            << "' (client " << client_id_ << ")" << std::endl;
  authenticated_ = true;

  if (server_->login_callback_) {
    server_->login_callback_(client_id_, username_, endpoint_);
  }

  SendLoginResponse(true, client_id_, server_->GetUDPPort());
}

void ProcessPacketTCP::SendLoginResponse(bool success, uint32_t player_id,
                                         uint16_t udp_port) {
  std::vector<uint8_t> response;

  response.push_back(0x02);
  response.push_back(0x01);

  response.push_back(0x00);
  response.push_back(0x00);
  response.push_back(0x00);
  response.push_back(0x09);

  response.push_back(0x01);
  response.push_back((player_id >> 8) & 0xFF);
  response.push_back(player_id & 0xFF);

  response.push_back(0x00);
  response.push_back(0x00);
  response.push_back(0x00);
  response.push_back(0x01);

  response.push_back((udp_port >> 8) & 0xFF);
  response.push_back(udp_port & 0xFF);

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

  response.push_back(0x02);
  response.push_back(0x01);

  uint32_t payload_size = 4 + message.size();
  response.push_back((payload_size >> 24) & 0xFF);
  response.push_back((payload_size >> 16) & 0xFF);
  response.push_back((payload_size >> 8) & 0xFF);
  response.push_back(payload_size & 0xFF);

  response.push_back(0x00);
  response.push_back((error_code >> 8) & 0xFF);
  response.push_back(error_code & 0xFF);
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
        Close();
      });
}
