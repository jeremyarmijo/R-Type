#include "network/Decoder.hpp"

#include <cstring>
#include <iostream>
#include <vector>

Decoder::Decoder() {
  handlers.fill(nullptr);

  // -------------------------
  // TCP Messages
  // -------------------------
  handlers[0x01] = DecodeLOGIN_REQUEST;
  handlers[0x02] = DecodeLOGIN_RESPONSE;
  handlers[0x03] = DecodeSIGNUP_REQUEST;
  handlers[0x04] = DecodeSIGNUP_RESPONSE;
  handlers[0x05] = DecodeLOGOUT;
  handlers[0x06] = DecodeLOBBY_LIST_REQUEST;
  handlers[0x07] = DecodeLOBBY_LIST_RESPONSE;
  handlers[0x08] = DecodeLOBBY_JOIN;
  handlers[0x09] = DecodeLOBBY_UPDATE;
  handlers[0x0A] = DecodePLAYER_READY;
  handlers[0x0B] = DecodeLOBBY_LEAVE;
  handlers[0x0C] = DecodeCHAT_MESSAGE;
  handlers[0x0D] = DecodeGAME_LOADING;
  handlers[0x0E] = DecodePLAYER_END_LOADING;
  handlers[0x0F] = DecodeGAME_START;
  handlers[0x10] = DecodeGAME_END;
  handlers[0x11] = DecodePLAYER_DISCONNECT;
  handlers[0x12] = DecodeERROR;
  handlers[0x13] = DecodeCHUNK_REQUEST;
  handlers[0x14] = DecodeCHUNK_DATA;

  // -------------------------
  // UDP Messages
  // -------------------------
  handlers[0x20] = DecodePLAYER_INPUT;
  handlers[0x21] = DecodeGAME_STATE;
  handlers[0x22] = DecodeENTITY_SPAWN;
  handlers[0x23] = DecodeENTITY_DESTROY;
  handlers[0x24] = DecodePLAYER_HIT;
  handlers[0x25] = DecodePOWERUP_COLLECTED;
  handlers[0x26] = DecodeFORCE_UPDATE;
  handlers[0x27] = DecodeBOSS_SPAWN;
  handlers[0x28] = DecodeBOSS_UPDATE;
  handlers[0x29] = DecodeSCROLLING_UPDATE;
  handlers[0x2A] = DecodeACK;
  handlers[0x2B] = DecodeCHUNK_UNLOAD;
  handlers[0x2C] = DecodeCHUNK_TILE_UPDATE;
  handlers[0x2D] = DecodeCHUNK_VISIBILITY;
}

Event Decoder::decode(const std::vector<uint8_t> &packet) {
  if (packet.size() < 6) {
    return {};
  }

  uint8_t type = packet[0];
  uint8_t flag = packet[1];

  uint32_t length;
  memcpy(&length, &packet[2], 4);

  const uint8_t *payload = packet.data() + 6;

  if (!handlers[type]) {
    return {type, {}};
  }

  return handlers[type](payload, length, type, flag);
}
