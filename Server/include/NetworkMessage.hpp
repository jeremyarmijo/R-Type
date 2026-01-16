#pragma once
#include <cstdint>
#include <vector>

/**
 * @struct NetworkMessage
 * @brief Represents a network message exchanged between server and clients
 *
 * Contains the client identifier, raw message data, and timestamp
 * for tracking and debugging purposes.
 */
struct NetworkMessage {
  uint16_t client_id;
  std::vector<uint8_t> data;

  NetworkMessage() : client_id(0) {}
};
