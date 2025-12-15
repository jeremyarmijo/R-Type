#pragma once
#include <iostream>
#include <unordered_set>
#include <vector>

#include "settings/PlayerSettings.hpp"

class MultiplayerSkinManager {
 private:
  PlayerSkin m_localPlayerSkin;
  std::vector<PlayerSkin> m_otherPlayerSkins;
  std::unordered_set<int> m_usedSkins;

 public:
  MultiplayerSkinManager() : m_localPlayerSkin(PlayerSkin::BLUE) {}

  void SetLocalPlayerSkin(PlayerSkin skin) {
    m_localPlayerSkin = skin;
    m_usedSkins.clear();
    m_usedSkins.insert(static_cast<int>(skin));
  }

  PlayerSkin AssignSkinForPlayer(int playerIndex) {
    if (playerIndex < static_cast<int>(m_otherPlayerSkins.size())) {
      return m_otherPlayerSkins[playerIndex];
    }

    PlayerSkin assignedSkin = FindUnusedSkin();
    if (playerIndex >= static_cast<int>(m_otherPlayerSkins.size())) {
      m_otherPlayerSkins.resize(playerIndex + 1);
    }

    m_otherPlayerSkins[playerIndex] = assignedSkin;
    m_usedSkins.insert(static_cast<int>(assignedSkin));

    std::cout << "Assigned skin " << PlayerSettings::GetSkinName(assignedSkin)
              << " to player " << playerIndex << std::endl;

    return assignedSkin;
  }

  void RemovePlayer(int playerIndex) {
    if (playerIndex < static_cast<int>(m_otherPlayerSkins.size())) {
      int skinInt = static_cast<int>(m_otherPlayerSkins[playerIndex]);
      m_usedSkins.erase(skinInt);
    }
  }

  void Clear() {
    m_otherPlayerSkins.clear();
    m_usedSkins.clear();
    m_usedSkins.insert(static_cast<int>(m_localPlayerSkin));
  }

 private:
  PlayerSkin FindUnusedSkin() {
    for (int i = 0; i < 5; ++i) {
      if (m_usedSkins.find(i) == m_usedSkins.end()) {
        return static_cast<PlayerSkin>(i);
      }
    }

    // If all skins are used (more than 5 players), just cycle through
    // This shouldn't happen with 5 skins and max 4 players, but just in case
    return static_cast<PlayerSkin>(m_otherPlayerSkins.size() % 5);
  }
};
