#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "scene/Scene.hpp"
#include "Player/PlayerEntity.hpp"
#include "physics/Physics2D.hpp"

// Simplified fighter states
struct FighterState {
    enum State {
        IDLE,
        WALKING,
        JUMPING,
        ATTACKING_LIGHT,
        ATTACKING_HEAVY
    };
    
    State currentState = IDLE;
    float stateTimer = 0.0f;
    bool facingRight = true;
};

struct FighterStats {
    float moveSpeed = 200.0f;
    float jumpForce = 400.0f;
    bool isGrounded = true;
};

struct HitBox {
    Vector2 offset;
    float width;
    float height;
    int damage;
    float stunDuration;
    bool active;
    
    HitBox() : offset{0,0}, width(0), height(0), damage(0), 
               stunDuration(0), active(false) {}
};

class SecondGame : public Scene {
private:
    std::unordered_map<uint16_t, Entity> m_players;
    
    const float ARENA_LEFT = 50.0f;
    const float ARENA_RIGHT = 750.0f;
    const float ARENA_FLOOR = 500.0f;
    
    int roundNumber = 1;
    const int MAX_ROUNDS = 3;
    std::unordered_map<uint16_t, int> roundWins;
    bool roundActive = false;
    float roundTimer = 99.0f;
    float roundStartDelay = 3.0f;
    
    void ReceivePlayerInputs();
    void UpdateFighterStates(float deltaTime);
    void Movement(float deltaTime);
    void CheckCombat();
    void CheckRoundEnd();
    void StartNewRound();
    void BuildCurrentState();

public:
    SecondGame();
    
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
