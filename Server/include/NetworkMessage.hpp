#pragma once
#include <vector>
#include <cstdint>

struct NetworkMessage {
    uint32_t client_id;              // 0 pour serveur
    std::vector<uint8_t> data;
    uint64_t timestamp;              // Pour latence/ordering
    
    NetworkMessage() : client_id(0), timestamp(0) {}
};
