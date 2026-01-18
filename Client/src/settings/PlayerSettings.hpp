#pragma once
#include <fstream>
#include <iostream>
#include <string>

enum class PlayerSkin { BLUE = 0, PINK = 1, GREEN = 2, RED = 3, DARKBLUE = 4 };

class PlayerSettings {
 private:
  PlayerSkin m_selectedSkin;
  std::string m_configFile;

 public:
  PlayerSettings(const std::string& configFile =
                     "../src/config/player_settings.cfg")
      : m_selectedSkin(PlayerSkin::BLUE), m_configFile(configFile) {
    LoadFromFile();
  }

  PlayerSkin GetSelectedSkin() const { return m_selectedSkin; }

  void SetSelectedSkin(PlayerSkin skin) {
    m_selectedSkin = skin;
    std::cout << "Selected skin: " << GetSkinName(skin) << std::endl;
  }

  std::string GetSelectedSkinAnimation() const {
    return GetSkinAnimation(m_selectedSkin);
  }

  static std::string GetSkinAnimation(PlayerSkin skin) {
    switch (skin) {
      case PlayerSkin::BLUE:
        return "blue_player";
      case PlayerSkin::PINK:
        return "pink_player";
      case PlayerSkin::GREEN:
        return "green_player";
      case PlayerSkin::RED:
        return "red_player";
      case PlayerSkin::DARKBLUE:
        return "darkblue_player";
      default:
        return "blue_player";
    }
  }

  static std::string GetSkinName(PlayerSkin skin) {
    switch (skin) {
      case PlayerSkin::BLUE:
        return "Blue";
      case PlayerSkin::PINK:
        return "Pink";
      case PlayerSkin::GREEN:
        return "Green";
      case PlayerSkin::RED:
        return "Red";
      case PlayerSkin::DARKBLUE:
        return "Dark Blue";
      default:
        return "Blue";
    }
  }

  static PlayerSkin GetNextSkin(PlayerSkin current) {
    int next = (static_cast<int>(current) + 1) % 5;
    return static_cast<PlayerSkin>(next);
  }

  static PlayerSkin GetPreviousSkin(PlayerSkin current) {
    int prev = (static_cast<int>(current) - 1);
    if (prev < 0) prev = 4;
    return static_cast<PlayerSkin>(prev);
  }

  bool SaveToFile() const {
    std::ofstream file(m_configFile);
    if (!file.is_open()) {
      std::cerr << "Failed to save player settings to " << m_configFile
                << std::endl;
      return false;
    }

    file << static_cast<int>(m_selectedSkin) << "\n";
    file.close();

    std::cout << "Player settings saved" << std::endl;
    return true;
  }

  bool LoadFromFile() {
    std::ifstream file(m_configFile);
    if (!file.is_open()) {
      std::cout << "No player settings file found, using defaults" << std::endl;
      return false;
    }

    int skinInt;
    if (file >> skinInt) {
      m_selectedSkin = static_cast<PlayerSkin>(skinInt);
      std::cout << "Loaded player skin: " << GetSkinName(m_selectedSkin)
                << std::endl;
    }

    file.close();
    return true;
  }
};
