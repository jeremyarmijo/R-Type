#include "network/UDPServer.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

UDPServer::UDPServer(asio::io_context& io_context, uint16_t port,
                     const std::string& host)
    : socket_(io_context,
              asio::ip::udp::endpoint(asio::ip::make_address(host), port)) {
  std::cout << "[UDPServer] Listening on " << host << ":" << port << std::endl;
  std::cout << "[UDPServer] Socket opened: "
            << socket_.local_endpoint().address().to_string() << std::endl;
  StartReceive();
}

UDPServer::~UDPServer() { Close(); }

void UDPServer::SetReceiveCallback(ReceiveCallback callback) {
  receive_callback_ = std::move(callback);
}

void UDPServer::StartReceive() {
  socket_.async_receive_from(
      asio::buffer(recv_buffer_), remote_endpoint_,
      [this](const asio::error_code& error, std::size_t bytes) {
        HandleReceive(error, bytes);
      });
}

void UDPServer::HandleReceive(const asio::error_code& error,
                              std::size_t bytes_transferred) {
  if (!error && bytes_transferred > 0) {
    if (receive_callback_) {
      std::vector<uint8_t> data(recv_buffer_.begin(),
                                recv_buffer_.begin() + bytes_transferred);
      receive_callback_(data, remote_endpoint_);
    }
    StartReceive();
  } else if (error != asio::error::operation_aborted) {
    std::cerr << "[UDPServer] Receive error: " << error.message() << std::endl;
    StartReceive();
  }
}

void UDPServer::SendTo(const std::vector<uint8_t>& data,
                       const asio::ip::udp::endpoint& endpoint) {
  if (!socket_.is_open()) return;

  socket_.async_send_to(asio::buffer(data), endpoint,
                        [](const asio::error_code& error, std::size_t) {
                          if (error) {
                            std::cerr
                                << "[UDPServer] Send error: " << error.message()
                                << std::endl;
                          }
                        });
}

void UDPServer::Close() {
  if (socket_.is_open()) {
    asio::error_code ec;
    socket_.close(ec);
  }
}
