#pragma once
#include "protocol/Protocol.hpp"
#include "protocol/Packets.hpp"
#include <vector>
#include <cstring>

class PacketSerializer {
public:
    // Création d'un paquet
    template<typename T>
    static std::vector<uint8_t> Serialize(Protocol::PacketType type, 
                                          const T& payload,
                                          uint32_t sequence = 0) {
        std::vector<uint8_t> packet(sizeof(Protocol::PacketHeader) + sizeof(T));
        
        Protocol::PacketHeader header;
        header.magic = Protocol::MAGIC_NUMBER;
        header.type = static_cast<uint8_t>(type);
        header.flags = 0;
        header.sequence = sequence;
        header.size = sizeof(T);
        
        std::memcpy(packet.data(), &header, sizeof(header));
        std::memcpy(packet.data() + sizeof(header), &payload, sizeof(T));
        
        return packet;
    }
    
    // Lecture d'un paquet
    static bool Deserialize(const std::vector<uint8_t>& data,
                           Protocol::PacketHeader& header) {
        if (data.size() < sizeof(Protocol::PacketHeader)) {
            return false;
        }
        
        std::memcpy(&header, data.data(), sizeof(header));
        
        if (header.magic != Protocol::MAGIC_NUMBER) {
            return false; // Paquet invalide
        }
        
        return true;
    }
    
    template<typename T>
    static bool DeserializePayload(const std::vector<uint8_t>& data, T& payload) {
        if (data.size() < sizeof(Protocol::PacketHeader) + sizeof(T)) {
            return false;
        }
        
        std::memcpy(&payload, data.data() + sizeof(Protocol::PacketHeader), sizeof(T));
        return true;
    }
};
