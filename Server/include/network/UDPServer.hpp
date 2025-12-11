#pragma once
#include <array>
#include <functional>
#include <memory>
#include <vector>
#include <asio.hpp>

class UDPServer {
 public:
  using ReceiveCallback = std::function<void(const std::vector<uint8_t>&,
                                             const asio::ip::udp::endpoint&)>;

  UDPServer(asio::io_context& io_context, uint16_t port);
  ~UDPServer();

  void SetReceiveCallback(ReceiveCallback callback);
  void SendTo(const std::vector<uint8_t>& data,
              const asio::ip::udp::endpoint& endpoint);

  bool IsOpen() const { return socket_.is_open(); }
  void Close();

 private:
  void StartReceive();
  void HandleReceive(const asio::error_code& error,
                     std::size_t bytes_transferred);

  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint remote_endpoint_;
  std::array<uint8_t, 1400> recv_buffer_;
  ReceiveCallback receive_callback_;
};
