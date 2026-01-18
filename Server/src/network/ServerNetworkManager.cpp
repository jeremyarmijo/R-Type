#include "network/ServerNetworkManager.hpp"

#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "network/DecodeFunc.hpp"
#include "network/EncodeFunc.hpp"
#include "db/Database.hpp"

ServerNetworkManager::ServerNetworkManager() {
  SetupEncoder(encode);
  SetupDecoder(decode);
  db = std::make_unique<Database>();
  if (db->Open("r-type.db")) {
    std::cout << "[Database] Opened r-type.db successfully." << std::endl;
  } else {
    throw std::runtime_error("[Database] Failed to open r-type.db.");
  }

  if (db->ExecuteQuery("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT NOT NULL, score INTEGER DEFAULT 0);")) {
    std::cout << "[Database] Users table ensured." << std::endl;
  } else {
    throw std::runtime_error("[Database] Failed to create users table.");
  }
  db->AddUser("testuser", "password123", 100);
  std::string password; int score;
  db->GetUser("testuser", password, score);
  std::cout << "User: testuser, Password: " << password << ", Score: " << score << std::endl;
};

ServerNetworkManager::~ServerNetworkManager() {
  Shutdown();
  if (db) {
    db->Close();
  }
}

bool ServerNetworkManager::Initialize(uint16_t tcp_port, uint16_t udp_port,
                                      const std::string &host) {
  try {
    work_guard_ = std::make_unique<asio::io_context::work>(io_context_);
    tcp_server_ = std::make_unique<TCPServer>(io_context_, tcp_port, host);
    tcp_server_->SetMessageCallback(
        [this](uint32_t client_id, const std::vector<uint8_t> &data) {
          OnReceiveTCP(client_id, data);
        });
    tcp_server_->SetUDPPort(udp_port);
    tcp_server_->SetUDPPort(udp_port);

    tcp_server_->SetLoginCallback(
        [this](uint32_t client_id, const std::string &username,
               const asio::ip::tcp::endpoint &endpoint,
               const std::vector<uint8_t> &data) {
          OnTCPLogin(client_id, username, endpoint, data);
        });
    tcp_server_->SetDisconnectCallback(
        [this](uint32_t client_id) { OnTCPDisconnect(client_id); });
    udp_server_ = std::make_unique<UDPServer>(io_context_, udp_port, host);

    udp_server_->SetReceiveCallback(
        [this](const auto &data, const auto &endpoint) {
          OnReceive(data, endpoint);
        });

    timeout_timer_ = std::make_unique<asio::steady_timer>(io_context_);
    // CheckClientTimeouts();

    io_thread_ = std::thread([this]() { IOThreadFunc(); });

    std::cout << "[ServerNetworkManager] Initialized TCP:" << tcp_port
              << " UDP:" << udp_port << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[ServerNetworkManager] Initialization failed: " << e.what()
              << std::endl;
    return false;
  }
}

template <typename T>
T bytes_to_type(const uint8_t *data, std::function<T(const T)> converter) {
  T tmp;
  std::memcpy(&tmp, data, sizeof(T));
  return converter(tmp);
}

void ServerNetworkManager::Shutdown() {
  if (work_guard_) {
    work_guard_.reset();
    io_context_.stop();
  }

  if (io_thread_.joinable()) {
    io_thread_.join();
  }

  tcp_server_.reset();
  udp_server_.reset();
  std::cout << "[ServerNetworkManager] Shutdown complete" << std::endl;
}

void ServerNetworkManager::IOThreadFunc() {
  std::cout << "[ServerNetworkManager] IO thread started" << std::endl;
  io_context_.run();
  std::cout << "[ServerNetworkManager] IO thread stopped" << std::endl;
}

void ServerNetworkManager::OnReceive(const std::vector<uint8_t> &data,
                                     const asio::ip::udp::endpoint &sender) {
  if (data.size() < 6) return;

  auto client = client_manager_.GetUDPClientByEndpoint(sender);
  Event evt = decode.decode(data);

  if (!client) {
    const AUTH *auth = std::get_if<AUTH>(&evt.data);
    if (!auth || evt.type != EventType::AUTH) return;

    client = client_manager_.GetClient(auth->playerId);
    if (!client) return;

    client_manager_.AssociateUDPEndpoint(auth->playerId, sender);
    std::cout << "[Network] UDP associated for client " << auth->playerId
              << "\n";
  }

  uint16_t last_seq = client->GetLastReceivedSeq();
  if (static_cast<int16_t>(evt.seqNum - last_seq) <= 0) {
    return;
  }
  client->UpdateLocalSequence(evt.seqNum);
  client->UpdateRemoteAck(evt.ack, evt.ack_bits);

  {
    std::lock_guard<std::mutex> lock(client->history_mutex);
    client->history.erase(evt.ack);
    for (int i = 0; i < 32; ++i) {
      if (evt.ack_bits & (1 << i)) {
        client->history.erase(static_cast<uint16_t>(evt.ack - (i + 1)));
      }
    }
  }

  client->UpdateLastSeen();
  client->IncrementPacketsReceived();

  NetworkMessage msg;
  msg.client_id = client->GetId();
  msg.data = data;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    incoming_messages_.push(std::move(msg));
  }
}

void ServerNetworkManager::OnReceiveTCP(uint32_t client_id,
                                        const std::vector<uint8_t> &data) {
  auto client = client_manager_.GetClient(client_id);
  if (!client) return;

  client->UpdateLastSeen();

  NetworkMessage msg;
  msg.client_id = static_cast<uint16_t>(client_id);
  msg.data = data;

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    incoming_messages_.push(std::move(msg));
  }
}

void ServerNetworkManager::Update() {
  std::queue<NetworkMessage> messages;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    messages.swap(incoming_messages_);
  }

  while (!messages.empty()) {
    if (message_callback_) {
      message_callback_(messages.front());
    }
    messages.pop();
  }

  std::queue<ConnectionEvent> events;
  {
    std::lock_guard<std::mutex> lock(events_mutex_);
    events.swap(connection_events_);
  }
  while (!events.empty()) {
    auto &event = events.front();

    if (event.type == ConnectionEvent::CONNECTED) {
      if (connection_callback_) {
        connection_callback_(event.client_id);
      }
    } else if (event.type == ConnectionEvent::DISCONNECTED) {
      if (disconnection_callback_) {
        disconnection_callback_(event.client_id);
      }
    }

    events.pop();
  }

  auto now = std::chrono::steady_clock::now();
  for (auto &client : client_manager_.GetAllClients()) {
    if (!client) continue;
    std::lock_guard<std::mutex> lock(client->history_mutex);
    for (auto it = client->history.begin(); it != client->history.end();) {
      auto &packet = it->second;

      if (packet.retry_count >= 15) {
        it = client->history.erase(it);
        continue;
      }

      if (now - packet.last_sent > std::chrono::milliseconds(100)) {
        std::cout << "[RETRY] Resending critical packet seq: " << it->first
                  << std::endl;
        udp_server_->SendTo(packet.data, client->GetUDPEndpoint());
        packet.last_sent = now;
        packet.retry_count++;
      }
      ++it;
    }
  }
}

void ServerNetworkManager::SendTo(const NetworkMessage &msg, Action ac) {
  auto client = client_manager_.GetClient(msg.client_id);
  if (!client) return;

  size_t protocol = UseUdp(ac.type);

  if ((protocol == 0 || protocol == 1) && client->HasUDPEndpoint()) {
    uint16_t seq = client->GetNextOutSeq();
    uint16_t ack = client->GetLastReceivedSeq();
    uint32_t bits = client->GetLocalAckBits();

    std::vector<uint8_t> finalPacket =
        encode.encode(ac, protocol, seq, ack, bits);
    if (protocol == 1) {
      std::lock_guard<std::mutex> lock(client->history_mutex);
      client->history[seq] = {seq, finalPacket,
                              std::chrono::steady_clock::now(), 1};
    }
    udp_server_->SendTo(finalPacket, client->GetUDPEndpoint());
    client->IncrementPacketsSent();
  } else if (tcp_server_) {
    std::vector<uint8_t> finalPacket = encode.encode(ac, 2, 0, 0, 0);
    tcp_server_->SendTo(finalPacket, client->GetId());
    client->IncrementPacketsSent();
  }
}

void ServerNetworkManager::BroadcastLobbyUDP(
    Action ac, std::vector<std::tuple<uint16_t, bool, std::string>> &ids) {
  size_t protocol = UseUdp(ac.type);

  for (auto &id : ids) {
    uint16_t clientId = std::get<0>(id);
    auto client = client_manager_.GetClient(clientId);

    if (client && udp_server_ && client->HasUDPEndpoint()) {
      uint16_t seq = client->GetNextOutSeq();
      uint16_t ack = client->GetLastReceivedSeq();
      uint32_t bits = client->GetLocalAckBits();

      std::vector<uint8_t> finalPacket =
          encode.encode(ac, protocol, seq, ack, bits);

      if (protocol == 1) {
        std::lock_guard<std::mutex> lock(client->history_mutex);
        client->history[seq] = {seq, finalPacket,
                                std::chrono::steady_clock::now(), 1};
      }

      udp_server_->SendTo(finalPacket, client->GetUDPEndpoint());
      client->IncrementPacketsSent();
    }
  }
}

void ServerNetworkManager::BroadcastLobbyTCP(
    Action ac, std::vector<std::tuple<uint16_t, bool, std::string>> &ids) {
  for (auto &id : ids) {
    uint16_t clientId = std::get<0>(id);
    auto client = client_manager_.GetClient(clientId);

    if (client && tcp_server_) {
      std::vector<uint8_t> finalPacket = encode.encode(ac, 2, 0, 0, 0);
      tcp_server_->SendTo(finalPacket, client->GetId());
      client->IncrementPacketsSent();
    }
  }
}

void ServerNetworkManager::CheckClientTimeouts() {
  auto timed_out = client_manager_.CheckTimeouts(CLIENT_TIMEOUT);

  if (GameStarted == false) {
    timeout_timer_->expires_after(std::chrono::seconds(1));
    timeout_timer_->async_wait(
        [this](const asio::error_code &) { CheckClientTimeouts(); });
    return;
  }
  for (uint32_t client_id : timed_out) {
    std::cout << "[ServerNetworkManager] Client " << client_id << " timed out"
              << std::endl;
    if (tcp_server_) tcp_server_->DisconnectClient(client_id);
    client_manager_.RemoveClient(client_id);
  }

  timeout_timer_->expires_after(std::chrono::seconds(1));
  timeout_timer_->async_wait(
      [this](const asio::error_code &) { CheckClientTimeouts(); });
}

void ServerNetworkManager::SetMessageCallback(MessageCallback callback) {
  message_callback_ = std::move(callback);
}

void ServerNetworkManager::SetConnectionCallback(ConnectionCallback callback) {
  connection_callback_ = std::move(callback);
}

void ServerNetworkManager::SetDisconnectionCallback(
    DisconnectionCallback callback) {
  disconnection_callback_ = std::move(callback);
}

void ServerNetworkManager::OnTCPLogin(
    uint32_t client_id, const std::string &username,
    const asio::ip::tcp::endpoint &tcp_endpoint,
    const std::vector<uint8_t> &data) {
  auto client =
      client_manager_.AddClientFromTCP(tcp_endpoint, username, client_id);

  if (client) {
    std::cout << "[ServerNetworkManager] Client " << client_id << " ("
              << username << ") logged in via TCP" << std::endl;

    ConnectionEvent event{ConnectionEvent::CONNECTED, client_id};
    {
      std::lock_guard<std::mutex> lock(events_mutex_);
      connection_events_.push(event);
    }
    OnReceiveTCP(client_id, data);
  }
}

void ServerNetworkManager::OnTCPDisconnect(uint32_t client_id) {
  std::cout << "[ServerNetworkManager] Client " << client_id
            << " disconnected from TCP" << std::endl;

  ConnectionEvent event{ConnectionEvent::DISCONNECTED, client_id};
  {
    std::lock_guard<std::mutex> lock(events_mutex_);
    connection_events_.push(event);
  }

  client_manager_.RemoveClient(client_id);
}

bool ServerNetworkManager::GetUser(const std::string& username, std::string& password, int& score) {
  if (!db) {
    std::cerr << "[ServerNetworkManager] Database not initialized" << std::endl;
    return false;
  }
  return db->GetUser(username, password, score);
}

bool ServerNetworkManager::AddUser(const std::string& username, const std::string& password, int score) {
  if (!db) {
    std::cerr << "[ServerNetworkManager] Database not initialized" << std::endl;
    return false;
  }
  return db->AddUser(username, password, score);
}

#ifdef _WIN32
__declspec(dllexport) INetworkManager *EntryPointLib() {
  return new ServerNetworkManager();
}
#else
INetworkManager *EntryPointLib() { return new ServerNetworkManager(); }
#endif
