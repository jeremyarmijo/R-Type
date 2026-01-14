#pragma once
#include <cstdint>
#include <vector>

#include "network/Decoder.hpp"
#include "network/Event.hpp"

bool checkHeader(const std::vector<uint8_t>& packet, size_t& offset,
                 uint32_t& payloadLength, uint16_t& seq, uint16_t& ack);

Event DecodeLOGIN_REQUEST(const std::vector<uint8_t>& packet);
Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet);
Event DecodeGAME_START(const std::vector<uint8_t>& packet);
Event DecodeGAME_END(const std::vector<uint8_t>& packet);
Event DecodeERROR(const std::vector<uint8_t>& packet);
Event DecodePLAYER_INPUT(const std::vector<uint8_t>& packet);
Event DecodeGAME_STATE(const std::vector<uint8_t>& packet);
Event DecodeAUTH(const std::vector<uint8_t>& packet);
Event DecodeBOSS_SPAWN(const std::vector<uint8_t>& packet);
Event DecodeBOSS_UPDATE(const std::vector<uint8_t>& packet);
Event DecodeENEMY_HIT(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_CREATE(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_JOIN_REQUEST(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_JOIN_RESPONSE(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_LIST_REQUEST(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_LIST_RESPONSE(const std::vector<uint8_t>& packet);
Event DecodePLAYER_READY(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_UPDATE(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_LEAVE(const std::vector<uint8_t>& packet);
Event DecodeLOBBY_START(const std::vector<uint8_t>& packet);

void SetupDecoder(Decoder& decoder);
