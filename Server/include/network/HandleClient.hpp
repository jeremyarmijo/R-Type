#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <asio.hpp>


/**
 * @class HandleClient
 * @brief Represents a connected client with its state and endpoints
 *
 * Manages client information including TCP/UDP endpoints, authentication state,
 * activity tracking, and packet statistics.
 */
class HandleClient {
 public:
  /**
   * @brief Construct a new HandleClient
   * @param id Unique client identifier
   * @param tcp_endpoint Client's TCP endpoint
   * @param username Client's username
   */

  struct SentPacket {
    uint16_t seq;
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point last_sent;
    int retry_count;
  };
  std::map<uint16_t, SentPacket> history;
  std::mutex history_mutex;

  HandleClient(uint16_t id, const asio::ip::tcp::endpoint& tcp_endpoint,
               const std::string& username);

  /**
   * @brief Get client ID
   * @return Client identifier
   */
  uint16_t GetId() const { return id_; }

  /**
   * @brief Get client username
   * @return Client username
   */
  const std::string& GetUsername() const { return username_; }

  /**
   * @brief Get client TCP endpoint
   * @return TCP endpoint
   */
  const asio::ip::tcp::endpoint& GetTCPEndpoint() const {
    return tcp_endpoint_;
  }

  /**
   * @brief Check if client has UDP endpoint set
   * @return true if UDP endpoint is set, false otherwise
   */
  bool HasUDPEndpoint() const { return udp_endpoint_.has_value(); }

  /**
   * @brief Get client UDP endpoint
   * @return UDP endpoint
   */
  const asio::ip::udp::endpoint& GetUDPEndpoint() const {
    return udp_endpoint_.value();
  }

  /**
   * @brief Set client UDP endpoint
   * @param endpoint UDP endpoint to set
   */
  void SetUDPEndpoint(const asio::ip::udp::endpoint& endpoint) {
    udp_endpoint_ = endpoint;
  }

  /**
   * @brief Check if client is authenticated
   * @return true if authenticated, false otherwise
   */
  bool IsAuthenticated() const { return is_authenticated_; }

  /**
   * @brief Set client authentication state
   * @param auth Authentication state
   */
  void SetAuthenticated(bool auth) { is_authenticated_ = auth; }

  /**
   * @brief Update last seen timestamp to current time
   */
  void UpdateLastSeen();

  /**
   * @brief Check if client has timed out
   * @param timeout Timeout duration
   * @return true if client has timed out, false otherwise
   */
  bool IsTimedOut(std::chrono::seconds timeout) const;

  /**
   * @brief Get number of packets sent to this client
   * @return Packet count
   */
  uint64_t GetPacketsSent() const { return packets_sent_; }

  /**
   * @brief Get number of packets received from this client
   * @return Packet count
   */
  uint64_t GetPacketsReceived() const { return packets_received_; }

  /**
   * @brief Increment sent packets counter
   */
  void IncrementPacketsSent() { ++packets_sent_; }

  /**
   * @brief Increment received packets counter
   */
  void IncrementPacketsReceived() { ++packets_received_; }

  uint16_t GetNextOutSeq() { return next_out_seq_num_++; }

  void UpdateRemoteAck(uint16_t ack, uint32_t ack_bits) {
    last_received_remote_ack_ = ack;
    remote_ack_bits_ = ack_bits;
  }
  uint16_t GetLastRemoteAck() const { return last_received_remote_ack_; }
  uint32_t GetRemoteAckBits() const { return remote_ack_bits_; }

  void UpdateLocalSequence(uint16_t new_seq);

  uint16_t GetLastReceivedSeq() const { return last_received_seq_from_client_; }

  uint32_t GetLocalAckBits() const { return local_ack_bits_for_client_; }

 private:
  uint16_t id_;                           ///< Unique client identifier
  std::string username_;                  ///< Client username
  asio::ip::tcp::endpoint tcp_endpoint_;  ///< TCP endpoint
  std::optional<asio::ip::udp::endpoint>
      udp_endpoint_;       ///< Optional UDP endpoint
  bool is_authenticated_;  ///< Authentication state
  std::chrono::steady_clock::time_point
      last_seen_;  ///< Last activity timestamp

  uint16_t next_out_seq_num_ = 1;
  uint16_t last_received_remote_ack_ = 0;
  uint32_t remote_ack_bits_ = 0;

  uint16_t last_received_seq_from_client_ = 0;
  uint32_t local_ack_bits_for_client_ = 0;

  uint64_t packets_sent_ = 0;      ///< Number of packets sent
  uint64_t packets_received_ = 0;  ///< Number of packets received
};
