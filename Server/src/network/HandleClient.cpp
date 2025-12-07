#include "network/HandleClient.hpp"

HandleClient::HandleClient(uint32_t id, const asio::ip::tcp::endpoint& tcp_endpoint, const std::string& username)
    : id_(id), username_(username), tcp_endpoint_(tcp_endpoint), is_authenticated_(true) {
    UpdateLastSeen();
}

void HandleClient::UpdateLastSeen() {
    last_seen_ = std::chrono::steady_clock::now();
}

bool HandleClient::IsTimedOut(std::chrono::seconds timeout) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_seen_);
    return elapsed > timeout;
}
