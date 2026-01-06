#pragma once
#include <arpa/inet.h>

#include <asio.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/ServerMacro.hpp"
#include "network/DecodeFunc.hpp"

class ProcessPacketTCP;

/**
 * @class TCPServer
 * @brief TCP server for handling reliable client connections
 *
 * Manages TCP connections, client authentication, and reliable message
 * delivery.
 */
class TCPServer {
 public:
  using LoginCallback = std::function<void(
      uint16_t client_id, const std::string& username,
      const asio::ip::tcp::endpoint& endpoint)>;  ///< Login callback type
  using DisconnectCallback =
      std::function<void(uint16_t client_id)>;  ///< Disconnect callback type

  TCPServer(asio::io_context& io_context, uint16_t port,
            const std::string& host = "0.0.0.0");
  ~TCPServer();

  /**
   * @brief Set callback for client login events
   * @param callback Function to call when client logs in
   */
  void SetLoginCallback(LoginCallback callback);

  /**
   * @brief Set callback for client disconnect events
   * @param callback Function to call when client disconnects
   */
  void SetDisconnectCallback(DisconnectCallback callback);

  /**
   * @brief Disconnect a specific client
   * @param client_id Client identifier
   */
  void DisconnectClient(uint32_t client_id);

  /**
   * @brief Get the UDP port number
   * @return UDP port number
   */
  uint16_t GetUDPPort() const { return udp_port_; }

  /**
   * @brief Set the UDP port number
   * @param port UDP port number
   */
  void SetUDPPort(uint16_t port) { udp_port_ = port; }

  /**
   * @brief Send data to a specific client
   * @param data Data bytes to send
   * @param client_id Target client identifier
   */
  void SendTo(const std::vector<uint8_t>& data, uint16_t client_id);

 private:
  friend class ProcessPacketTCP;

  /**
   * @brief Start accepting new client connections
   */
  void StartAccept();

  asio::ip::tcp::acceptor acceptor_;  ///< TCP acceptor for incoming connections
  LoginCallback login_callback_;      ///< Callback for login events
  DisconnectCallback disconnect_callback_;  ///< Callback for disconnect events

  std::unordered_map<uint16_t, std::shared_ptr<ProcessPacketTCP>>
      sessions_;                     ///< Active client sessions
  std::mutex process_packet_mutex_;  ///< Mutex for sessions access

  uint16_t next_client_id_;  ///< Next available client ID
  uint16_t udp_port_;        ///< Associated UDP port
};

/**
 * @class ProcessPacketTCP
 * @brief Handles TCP packet processing for a single client session
 *
 * Manages the lifecycle of a TCP connection, including reading headers,
 * payloads, authentication, and sending responses.
 */
class ProcessPacketTCP : public std::enable_shared_from_this<ProcessPacketTCP> {
 public:
  /**
   * @brief Construct a new ProcessPacketTCP session
   * @param socket TCP socket for this client
   * @param client_id Unique client identifier
   * @param server Pointer to parent TCPServer
   */
  ProcessPacketTCP(asio::ip::tcp::socket socket, uint16_t client_id,
                   TCPServer* server);

  /**
   * @brief Destroy the ProcessPacketTCP session
   */
  ~ProcessPacketTCP();

  /**
   * @brief Start processing packets for this session
   */
  void Start();

  /**
   * @brief Close the TCP connection
   */
  void Close();

  /**
   * @brief Send a packet to the client
   * @param data Data bytes to send
   */
  void SendPacket(const std::vector<uint8_t>& data);

  /**
   * @brief Get client ID
   * @return Client identifier
   */
  uint32_t GetClientId() const { return client_id_; }

  /**
   * @brief Get client endpoint
   * @return TCP endpoint of the client
   */
  const asio::ip::tcp::endpoint& GetEndpoint() const { return endpoint_; }

 private:
  /**
   * @brief Start reading packet header
   */
  void ReadHeader();

  /**
   * @brief Handle completed header read
   * @param error Error code if read failed
   * @param bytes_transferred Number of bytes read
   */
  void HandleReadHeader(const asio::error_code& error,
                        std::size_t bytes_transferred);

  /**
   * @brief Start reading packet payload
   * @param payload_size Size of payload to read
   */
  void ReadPayload(uint16_t payload_size);

  /**
   * @brief Handle completed payload read
   * @param error Error code if read failed
   * @param bytes_transferred Number of bytes read
   */
  void HandleReadPayload(const asio::error_code& error,
                         std::size_t bytes_transferred);

  /**
   * @brief Process login request from client
   */
  void ProcessLoginRequest();

  /**
   * @brief Process game start request
   */
  void ProcessingGameStart();

  /**
   * @brief Send login response to client
   * @param success Whether login was successful
   * @param player_id Assigned player ID
   * @param udp_port UDP port for game data
   */
  void SendLoginResponse(bool success, uint16_t player_id, uint16_t udp_port);

  asio::ip::tcp::socket socket_;      ///< TCP socket for this client
  asio::ip::tcp::endpoint endpoint_;  ///< Client endpoint
  uint16_t client_id_;                ///< Unique client identifier
  TCPServer* server_;                 ///< Pointer to parent server

  std::array<uint8_t, HEADER_SIZE>
      header_buffer_;                    ///< Buffer for packet header
  std::vector<uint8_t> payload_buffer_;  ///< Buffer for packet payload
  uint8_t current_msg_type_ = 0;  ///< Current message type being processed

  bool authenticated_ = false;  ///< Authentication state
  std::string username_;        ///< Client username
};
