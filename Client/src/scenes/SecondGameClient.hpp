#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Player/PlayerEntity.hpp"
#include "network/DataMask.hpp"
#include "systems/InputSystem.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "audio/AudioSubsystem.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "network/NetworkSubsystem.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"
#include "Helpers/EntityHelper.hpp"

// Fighter state for visual representation
struct ClientFighterState {
    enum AnimState {
        ANIM_IDLE,
        ANIM_WALK,
        ANIM_CROUCH,
        ANIM_JUMP,
        ANIM_ATTACK_LIGHT,
        ANIM_ATTACK_MEDIUM,
        ANIM_ATTACK_HEAVY,
        ANIM_BLOCK,
        ANIM_HIT,
        ANIM_KNOCKDOWN
    };
    
    AnimState currentAnim = ANIM_IDLE;
    bool facingRight = true;
    float animTimer = 0.0f;
};

class SecondGameClient : public Scene {
private:
    // Player management
    std::unordered_map<uint16_t, Entity> m_fighters;
    Entity m_localFighter;
    uint16_t m_localPlayerId;
    bool m_isSpectator = false;
    
    // Game state tracking
    std::unordered_map<uint16_t, GAME_STATE::PlayerState> m_fighterStates;
    
    // Interpolation for smooth movement
    struct InterpolatedState {
        Vector2 currentPos;
        Vector2 targetPos;
        float interpolationTime = 0.0f;
        float INTERPOLATION_DURATION = 0.1f;
    };
    std::unordered_map<uint16_t, InterpolatedState> m_fighterInterpolation;
    
    // UI Elements
    UIText* m_player1HealthText;
    UIText* m_player2HealthText;
    UIText* m_roundText;
    UIText* m_timerText;
    UIText* m_player1WinsText;
    UIText* m_player2WinsText;
    UIText* m_fightText;
    UIText* m_spectatorText;
    
    // Arena visuals
    Entity m_arenaBackground;
    Entity m_arenaFloor;
    std::vector<Entity> m_healthBars;
    
    // Round tracking
    int m_currentRound = 1;
    int m_roundTimer = 99;
    std::unordered_map<uint16_t, int> m_roundWins;
    bool m_roundStarting = false;
    float m_fightTextTimer = 0.0f;
    
    bool m_isInitialized = false;
    
    // Helper functions
    void LoadFighterTextures();
    void CreateFighterAnimations();
    void CreateArenaVisuals();
    void SpawnFighter(uint16_t playerId, Vector2 position, bool facingRight);
    void RemoveFighter(uint16_t playerId);
    void UpdateFighterPosition(uint16_t playerId, Vector2 targetPosition);
    void UpdateFighterAnimation(uint16_t playerId, uint8_t state, bool facingRight);
    void InterpolateFighters(float deltaTime);
    void UpdateUI();
    void GetGameEvents(float deltaTime);
    void UpdateFighters(const std::vector<GAME_STATE::PlayerState>& fighters, float deltaTime);
    
    std::string GetFighterTexture(uint8_t fighterType) const;
    std::string GetFighterAnimation(uint8_t state) const;

public:
    SecondGameClient();
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleEvent(SDL_Event& event) override;
    
    std::unordered_map<uint16_t, Entity> GetPlayers() override {
        return std::unordered_map<uint16_t, Entity>();
    }
};

extern "C" {
    Scene* CreateScene();
}
