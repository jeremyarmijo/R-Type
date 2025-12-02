#pragma once
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "Decoder.hpp"
#include "Event.hpp"

Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_RESPONSE;

  LOGIN_RESPONSE data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];

  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  data.success = packet[offset++];

  if (data.success == 1) {
    memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
    offset += sizeof(data.playerId);

    memcpy(&data.serverTick, &packet[offset], sizeof(data.serverTick));
    offset += sizeof(data.serverTick);

    memcpy(&data.udpPort, &packet[offset], sizeof(data.udpPort));
    offset += sizeof(data.udpPort);
  } else {
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    offset += sizeof(data.errorCode);

    uint8_t msgLen = packet[offset++];
    data.message =
        std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
    offset += msgLen;
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_LIST_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_LIST_RESPONSE;
  LOBBY_LIST_RESPONSE data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  uint8_t lobbyCount = packet[offset++];
  data.lobbies.reserve(lobbyCount);

  for (uint8_t i = 0; i < lobbyCount; ++i) {
    LOBBY_LIST_RESPONSE::Lobby lobby;
    memcpy(&lobby.lobbyId, &packet[offset], sizeof(lobby.lobbyId));
    offset += sizeof(lobby.lobbyId);
    
    uint8_t nameLen = packet[offset++];
    lobby.name = std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;
    
    lobby.playerCount = packet[offset++];
    lobby.maxPlayers = packet[offset++];
    lobby.hasStarted = packet[offset++];
    
    data.lobbies.push_back(lobby);
  }

  evt.data = data;
  return evt;
}

Event DecodeLOBBY_UPDATE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOBBY_UPDATE;
  LOBBY_UPDATE data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.lobbyId, &packet[offset], sizeof(data.lobbyId));
  offset += sizeof(data.lobbyId);

  uint8_t playerCount = packet[offset++];
  data.players.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    LOBBY_UPDATE::PlayerInfo player;
    memcpy(&player.playerId, &packet[offset], sizeof(player.playerId));
    offset += sizeof(player.playerId);
    
    uint8_t nameLen = packet[offset++];
    player.name = std::string(reinterpret_cast<const char*>(&packet[offset]), nameLen);
    offset += nameLen;
    
    player.ready = packet[offset++];
    data.players.push_back(player);
  }

  evt.data = data;
  return evt;
}

Event DecodeCHAT_MESSAGE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHAT_MESSAGE;
  CHAT_MESSAGE data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.senderId, &packet[offset], sizeof(data.senderId));
  offset += sizeof(data.senderId);

  uint16_t msgLen;
  memcpy(&msgLen, &packet[offset], sizeof(msgLen));
  offset += sizeof(msgLen);
  
  data.message = std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodeGAME_LOADING(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_LOADING;
  GAME_LOADING data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.mapId, &packet[offset], sizeof(data.mapId));
  offset += sizeof(data.mapId);
  data.gameMode = packet[offset++];
  data.difficulty = packet[offset++];
  memcpy(&data.mapWidth, &packet[offset], sizeof(data.mapWidth));
  offset += sizeof(data.mapWidth);
  memcpy(&data.mapHeight, &packet[offset], sizeof(data.mapHeight));
  offset += sizeof(data.mapHeight);
  memcpy(&data.chunkSize, &packet[offset], sizeof(data.chunkSize));
  offset += sizeof(data.chunkSize);
  memcpy(&data.tickStart, &packet[offset], sizeof(data.tickStart));
  offset += sizeof(data.tickStart);

  evt.data = data;
  return evt;
}

Event DecodeGAME_START(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_START;
  GAME_START data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.startTick, &packet[offset], sizeof(data.startTick));
  offset += sizeof(data.startTick);
  memcpy(&data.playerSpawnX, &packet[offset], sizeof(data.playerSpawnX));
  offset += sizeof(data.playerSpawnX);
  memcpy(&data.playerSpawnY, &packet[offset], sizeof(data.playerSpawnY));
  offset += sizeof(data.playerSpawnY);
  memcpy(&data.scrollSpeed, &packet[offset], sizeof(data.scrollSpeed));
  offset += sizeof(data.scrollSpeed);

  evt.data = data;
  return evt;
}

Event DecodeGAME_END(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::GAME_END;
  GAME_END data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  data.victory = packet[offset++];
  uint8_t playerCount = packet[offset++];
  data.scores.reserve(playerCount);

  for (uint8_t i = 0; i < playerCount; ++i) {
    GAME_END::Score score;
    memcpy(&score.playerId, &packet[offset], sizeof(score.playerId));
    offset += sizeof(score.playerId);
    memcpy(&score.score, &packet[offset], sizeof(score.score));
    offset += sizeof(score.score);
    memcpy(&score.kills, &packet[offset], sizeof(score.kills));
    offset += sizeof(score.kills);
    data.scores.push_back(score);
  }

  evt.data = data;
  return evt;
}

Event DecodePLAYER_DISCONNECT(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::PLAYER_DISCONNECT;
  PLAYER_DISCONNECT data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
  offset += sizeof(data.playerId);
  data.reason = packet[offset++];

  evt.data = data;
  return evt;
}

Event DecodeERROR(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::ERROR;
  ERROR data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
  offset += sizeof(data.errorCode);
  
  uint8_t msgLen = packet[offset++];
  data.message = std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  offset += msgLen;

  evt.data = data;
  return evt;
}

Event DecodeCHUNK_DATA(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHUNK_DATA;
  CHUNK_DATA data;
  size_t offset = 0;

  uint8_t msgType = packet[offset++];
  uint8_t flags = packet[offset++];
  uint32_t payloadLength = 0;
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  offset += 4;

  if (payloadLength != packet.size() - 6) {
    return Event{};
  }

  memcpy(&data.chunkX, &packet[offset], sizeof(data.chunkX));
  offset += sizeof(data.chunkX);
  memcpy(&data.chunkWidth, &packet[offset], sizeof(data.chunkWidth));
  offset += sizeof(data.chunkWidth);
  memcpy(&data.chunkHeight, &packet[offset], sizeof(data.chunkHeight));
  offset += sizeof(data.chunkHeight);

  uint32_t tileCount;
  memcpy(&tileCount, &packet[offset], sizeof(tileCount));
  offset += sizeof(tileCount);
  data.tiles.reserve(tileCount);

  for (uint32_t i = 0; i < tileCount; ++i) {
    CHUNK_DATA::Tile tile;
    memcpy(&tile.tileX, &packet[offset], sizeof(tile.tileX));
    offset += sizeof(tile.tileX);
    memcpy(&tile.tileY, &packet[offset], sizeof(tile.tileY));
    offset += sizeof(tile.tileY);
    tile.tileType = packet[offset++];
    tile.tileSprite = packet[offset++];
    tile.tileFlags = packet[offset++];
    tile.tileHealth = packet[offset++];
    data.tiles.push_back(tile);
  }

  evt.data = data;
  return evt;
}

void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
}
