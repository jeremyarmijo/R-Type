#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Player/PlayerEntity.hpp"
#include "systems/InputSystem.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "audio/AudioSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "network/NetworkSubsystem.hpp"
#include "settings/MultiplayerSkinManager.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "ui/UIManager.hpp"
#include "ui/UISolidColor.hpp"
#include "ui/UIText.hpp"
#include "components/TileMap.hpp"
#include "Helpers/EntityHelper.hpp"

class RtypeScene : public Scene {
 private:
  const float TIME_BETWEEN_LEVELS = 5.0f;
  std::vector<LevelComponent> levelsData;
  int currentLevelIndex = 0;
  bool waitingForNextLevel = false;
  float levelTransitionTimer = 0.0f;
  Entity currentLevelEntity;
  std::unordered_map<uint16_t, Entity> m_players;
  TileMap currentMap;
  Entity mapEntity;
  bool mapSent = false;
  std::unordered_map<uint16_t, uint32_t> playerScores;
  std::unordered_map<uint16_t, std::shared_ptr<GameState>> lastStates;
  std::unordered_map<uint16_t, int> playerStateCount;

  void ReceivePlayerInputs();
  void UpdateGameState(float deltaTime);
  void BuildCurrentState();

 public:
  RtypeScene();

  void OnEnter() override;

  void OnExit() override;

  void Update(float deltaTime) override;

  void Render() override {}

  void HandleEvent(SDL_Event& event) override;

  std::unordered_map<uint16_t, Entity> GetPlayers() override {
    return m_players;
  }
};

extern "C" {
    Scene* CreateScene();
}
