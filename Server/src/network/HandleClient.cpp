#include "network/HandleClient.hpp"

#include <string>

HandleClient::HandleClient(uint16_t id,
                           const asio::ip::tcp::endpoint& tcp_endpoint,
                           const std::string& username)
    : id_(id),
      username_(username),
      tcp_endpoint_(tcp_endpoint),
      is_authenticated_(true) {
  UpdateLastSeen();
}

void HandleClient::UpdateLastSeen() {
  last_seen_ = std::chrono::steady_clock::now();
}

bool HandleClient::IsTimedOut(std::chrono::seconds timeout) const {
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(now - last_seen_);
  return elapsed > timeout;
}

void HandleClient::UpdateLocalSequence(uint16_t new_seq) {
  if (new_seq == last_received_seq_from_client_) return;

  uint16_t diff = new_seq - last_received_seq_from_client_;

  if (diff < 32768) {
    if (diff < 32) {
      local_ack_bits_for_client_ <<= diff;
      local_ack_bits_for_client_ |= (1 << (diff - 1));
    } else {
      local_ack_bits_for_client_ = 0;
    }
    last_received_seq_from_client_ = new_seq;
  } else {
    uint16_t late_diff = last_received_seq_from_client_ - new_seq;
    if (late_diff <= 32) {
      local_ack_bits_for_client_ |= (1U << (late_diff - 1));
    }
  }
}
