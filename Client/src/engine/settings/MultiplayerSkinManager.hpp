#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "settings/PlayerSettings.hpp"

class MultiplayerSkinManager {
private:
  PlayerSkin m_localPlayerSkin;
  uint16_t m_localPlayerId;
  std::unordered_map<uint16_t, PlayerSkin> m_playerSkins;
  std::unordered_set<int> m_usedSkins;

public:
  MultiplayerSkinManager() 
    : m_localPlayerSkin(PlayerSkin::BLUE),
      m_localPlayerId(0) {}

  void SetLocalPlayerSkin(PlayerSkin skin, uint16_t playerId) {
    m_localPlayerSkin = skin;
    m_localPlayerId = playerId;
    m_usedSkins.clear();
    m_usedSkins.insert(static_cast<int>(skin));
    
    std::cout << "Local player (ID: " << playerId << ") using skin: " 
              << PlayerSettings::GetSkinName(skin) << std::endl;
  }

  PlayerSkin AssignSkinForPlayer(uint16_t playerId) {
    if (playerId == m_localPlayerId) {
      return m_localPlayerSkin;
    }

    auto it = m_playerSkins.find(playerId);
    if (it != m_playerSkins.end()) {
      return it->second;
    }

    PlayerSkin assignedSkin = FindUnusedSkin();
    m_playerSkins[playerId] = assignedSkin;
    m_usedSkins.insert(static_cast<int>(assignedSkin));

    std::cout << "Assigned skin " << PlayerSettings::GetSkinName(assignedSkin)
              << " to player ID " << playerId << std::endl;

    return assignedSkin;
  }

  bool HasPlayer(uint16_t playerId) const {
    return m_playerSkins.find(playerId) != m_playerSkins.end();
  }

  PlayerSkin GetPlayerSkin(uint16_t playerId) const {
    if (playerId == m_localPlayerId) {
      return m_localPlayerSkin;
    }

    auto it = m_playerSkins.find(playerId);
    if (it != m_playerSkins.end()) {
      return it->second;
    }

    return PlayerSkin::BLUE;
  }

  void RemovePlayer(uint16_t playerId) {
    auto it = m_playerSkins.find(playerId);
    if (it != m_playerSkins.end()) {
      int skinInt = static_cast<int>(it->second);
      m_usedSkins.erase(skinInt);
      m_playerSkins.erase(it);

      std::cout << "Removed player ID " << playerId 
                << ", freed skin for reuse" << std::endl;
    }
  }

  void Clear() {
    m_playerSkins.clear();
    m_usedSkins.clear();
    m_usedSkins.insert(static_cast<int>(m_localPlayerSkin));

    std::cout << "Cleared all player skin assignments" << std::endl;
  }

  size_t GetPlayerCount() const {
    return m_playerSkins.size();
  }

  void PrintDebug() const {
    std::cout << "\n=== Skin Manager Debug ===" << std::endl;
    std::cout << "Local Player (ID: " << m_localPlayerId << "): " 
              << PlayerSettings::GetSkinName(m_localPlayerSkin) << std::endl;
    std::cout << "Other Players:" << std::endl;
    for (const auto& [playerId, skin] : m_playerSkins) {
      std::cout << "  Player " << playerId << ": " 
                << PlayerSettings::GetSkinName(skin) << std::endl;
    }
    std::cout << "========================\n" << std::endl;
  }

private:
  PlayerSkin FindUnusedSkin() {
    for (int i = 0; i < 5; ++i) {
      if (m_usedSkins.find(i) == m_usedSkins.end()) {
        return static_cast<PlayerSkin>(i);
      }
    }

    return static_cast<PlayerSkin>(m_playerSkins.size() % 5);
  }
};
