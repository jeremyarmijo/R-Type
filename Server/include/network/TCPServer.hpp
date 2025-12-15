#pragma once
#include <arpa/inet.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <asio.hpp>

#include "network/DecodeFunc.hpp"

class ProcessPacketTCP;

class TCPServer {
 public:
  using LoginCallback =
      std::function<void(uint16_t client_id, const std::string& username,
                         const asio::ip::tcp::endpoint& endpoint)>;
  using DisconnectCallback = std::function<void(uint16_t client_id)>;

  TCPServer(asio::io_context& io_context, uint16_t port);
  ~TCPServer();

  void SetLoginCallback(LoginCallback callback);
  void SetDisconnectCallback(DisconnectCallback callback);

  void DisconnectClient(uint32_t client_id);
  uint16_t GetUDPPort() const { return udp_port_; }
  void SetUDPPort(uint16_t port) { udp_port_ = port; }

  void SendTo(const std::vector<uint8_t>& data, uint16_t client_id);

 private:
  friend class ProcessPacketTCP;

  void StartAccept();

  asio::ip::tcp::acceptor acceptor_;
  LoginCallback login_callback_;
  DisconnectCallback disconnect_callback_;

  std::unordered_map<uint16_t, std::shared_ptr<ProcessPacketTCP>> sessions_;
  std::mutex process_packet_mutex_;

  uint16_t next_client_id_;
  uint16_t udp_port_;
};

class ProcessPacketTCP : public std::enable_shared_from_this<ProcessPacketTCP> {
 public:
  ProcessPacketTCP(asio::ip::tcp::socket socket, uint16_t client_id,
                   TCPServer* server);
  ~ProcessPacketTCP();

  void Start();
  void Close();
  void SendPacket(const std::vector<uint8_t>& data);

  uint32_t GetClientId() const { return client_id_; }
  const asio::ip::tcp::endpoint& GetEndpoint() const { return endpoint_; }

 private:
  void ReadHeader();
  void push_buffer_uint16(std::vector<uint8_t>& buffer, uint16_t value);
  void push_buffer_uint32(std::vector<uint8_t>& buffer, uint32_t value);
  void HandleReadHeader(const asio::error_code& error,
                        std::size_t bytes_transferred);
  void ReadPayload(uint16_t payload_size);
  void HandleReadPayload(const asio::error_code& error,
                         std::size_t bytes_transferred);

  void ProcessLoginRequest();
  void ProcessingGameStart();
  void SendLoginResponse(bool success, uint16_t player_id, uint16_t udp_port);
  void SendLoginError(uint16_t error_code, const std::string& message);

  asio::ip::tcp::socket socket_;
  asio::ip::tcp::endpoint endpoint_;
  uint16_t client_id_;
  TCPServer* server_;

  std::array<uint8_t, 6> header_buffer_;
  std::vector<uint8_t> payload_buffer_;
  uint8_t current_msg_type_ = 0;

  bool authenticated_ = false;
  std::string username_;
};
