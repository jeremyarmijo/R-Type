#pragma once
#include <vector>
#include <array>
#include <functional>
#include "Event.hpp"

class Decoder {
public:
    Decoder();

    Event decode(const std::vector<uint8_t>& packet);

private:
    using DecodeFunc = std::function<Event(const uint8_t*, uint32_t, uint8_t, uint8_t)>;
    std::array<DecodeFunc, 256> handlers;

    // -------------------------
    // TCP Messages
    // -------------------------
    static Event DecodeLOGIN_REQUEST(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOGIN_RESPONSE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeSIGNUP_REQUEST(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeSIGNUP_RESPONSE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOGOUT(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOBBY_LIST_REQUEST(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOBBY_LIST_RESPONSE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOBBY_JOIN(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOBBY_UPDATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodePLAYER_READY(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeLOBBY_LEAVE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHAT_MESSAGE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeGAME_LOADING(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodePLAYER_END_LOADING(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeGAME_START(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeGAME_END(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodePLAYER_DISCONNECT(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeERROR(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHUNK_REQUEST(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHUNK_DATA(const uint8_t*, uint32_t, uint8_t, uint8_t);

    // -------------------------
    // UDP Messages
    // -------------------------
    static Event DecodePLAYER_INPUT(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeGAME_STATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeENTITY_SPAWN(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeENTITY_DESTROY(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodePLAYER_HIT(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodePOWERUP_COLLECTED(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeFORCE_UPDATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeBOSS_SPAWN(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeBOSS_UPDATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeSCROLLING_UPDATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeACK(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHUNK_UNLOAD(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHUNK_TILE_UPDATE(const uint8_t*, uint32_t, uint8_t, uint8_t);
    static Event DecodeCHUNK_VISIBILITY(const uint8_t*, uint32_t, uint8_t, uint8_t);
};
