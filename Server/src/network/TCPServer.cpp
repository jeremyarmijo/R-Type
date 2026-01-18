
#include "network/TCPServer.hpp"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../include/ServerMacro.hpp"
#include "network/EncodeFunc.hpp"

TCPServer::TCPServer(asio::io_context& io_context, uint16_t port,
                     const std::string& host)
    : acceptor_(io_context,
                asio::ip::tcp::endpoint(asio::ip::make_address(host), port)),
      next_client_id_(1),
      udp_port_(PORT_UDP_DEFAULT) {
  std::cout << "[TCPServer] Listening on " << host << ":" << port << std::endl;
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
                                   uint16_t client_id, TCPServer* server)
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

template <typename T>
T bytes_to_type(const uint8_t* data, std::function<T(const T)> converter) {
  T tmp;
  std::memcpy(&tmp, data, sizeof(T));
  return converter(tmp);
}

void ProcessPacketTCP::HandleReadHeader(const asio::error_code& error,
                                        std::size_t bytes_transferred) {
  if (error || bytes_transferred != HEADER_SIZE) {
    if (error != asio::error::eof && error != asio::error::operation_aborted) {
      std::cerr << "[ProcessPacketTCP] Header read error: " << error.message()
                << std::endl;
    }

    // if (server_->disconnect_callback_ && authenticated_) {
    //   server_->disconnect_callback_(client_id_);
    // }
    // return;
  }

  uint8_t type = header_buffer_[0];
  uint8_t flags = header_buffer_[1];
  uint32_t length = bytes_to_type<uint32_t>(&header_buffer_[2], ntohl);

  std::cout << "[ProcessPacketTCP] Received header: type=0x" << std::hex
            << static_cast<int>(type) << " flags=0x" << static_cast<int>(flags)
            << " length=" << std::dec << length << std::endl;

  current_msg_type_ = type;

  if (length > 0) {
    std::cout << "read payload of length " << length << std::endl;
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
                                         std::size_t) {
  if (error && error != asio::error::eof) {
    std::cerr << "[ProcessPacketTCP] Payload read error: " << error.message()
              << std::endl;
    return;
  }

  std::vector<uint8_t> full_packet;
  full_packet.insert(full_packet.end(), header_buffer_.begin(),
                     header_buffer_.end());
  full_packet.insert(full_packet.end(), payload_buffer_.begin(),
                     payload_buffer_.end());

  if (current_msg_type_ == 0x01 && server_->login_callback_) {
    std::string username(payload_buffer_.begin(), payload_buffer_.end());
    server_->login_callback_(client_id_, username, endpoint_, full_packet);
  }
  if (server_->GetMessageCallback()) {
    server_->GetMessageCallback()(client_id_, full_packet);
  }

  ReadHeader();
}

void TCPServer::SendTo(const std::vector<uint8_t>& data, uint16_t client_id) {
  std::shared_ptr<ProcessPacketTCP> session;

  {
    std::lock_guard<std::mutex> lock(process_packet_mutex_);
    auto it = sessions_.find(client_id);
    if (it == sessions_.end()) {
      std::cerr << "[TCPServer] Cannot send: unknown client " << client_id
                << std::endl;
      return;
    }
    session = it->second;
  }

  session->SendPacket(data);
}

void ProcessPacketTCP::SendPacket(const std::vector<uint8_t>& data) {
  if (!socket_.is_open()) {
    std::cerr << "[ProcessPacketTCP] Cannot send packet: socket closed"
              << std::endl;
    return;
  }
  auto self = shared_from_this();
  asio::async_write(socket_, asio::buffer(data),
                    [this, self](const asio::error_code& error, std::size_t) {
                      if (error) {
                        std::cerr << "[ProcessPacketTCP] Send error: "
                                  << error.message() << std::endl;
                      }
                    });
}
