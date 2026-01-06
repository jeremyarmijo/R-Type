#pragma once
#include <array>
#include <asio.hpp>
#include <functional>
#include <memory>
#include <vector>

/**
 * @class UDPServer
 * @brief UDP server for handling fast, unreliable game data transmission
 *
 * Manages UDP communication for real-time game state updates and player inputs.
 */
class UDPServer {
 public:
  using ReceiveCallback = std::function<void(
      const std::vector<uint8_t>&,
      const asio::ip::udp::endpoint&)>;  ///< Receive callback type

  UDPServer(asio::io_context& io_context, uint16_t port,
            const std::string& host = "0.0.0.0");
  ~UDPServer();

  /**
   * @brief Set callback for incoming UDP data
   * @param callback Function to call when data is received
   */
  void SetReceiveCallback(ReceiveCallback callback);

  /**
   * @brief Send data to a specific endpoint
   * @param data Data bytes to send
   * @param endpoint Target UDP endpoint
   */
  void SendTo(const std::vector<uint8_t>& data,
              const asio::ip::udp::endpoint& endpoint);

  /**
   * @brief Check if socket is open
   * @return true if socket is open, false otherwise
   */
  bool IsOpen() const { return socket_.is_open(); }

  /**
   * @brief Close the UDP socket
   */
  void Close();

 private:
  /**
   * @brief Start receiving UDP data asynchronously
   */
  void StartReceive();

  /**
   * @brief Handle completed receive operation
   * @param error Error code if receive failed
   * @param bytes_transferred Number of bytes received
   */
  void HandleReceive(const asio::error_code& error,
                     std::size_t bytes_transferred);

  asio::ip::udp::socket socket_;  ///< UDP socket
  asio::ip::udp::endpoint
      remote_endpoint_;  ///< Remote endpoint for current receive
  std::array<uint8_t, 1400> recv_buffer_;  ///< Receive buffer (MTU-safe size)
  ReceiveCallback receive_callback_;       ///< Callback for incoming data
};
