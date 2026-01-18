#include "scenes/SecondGame.hpp"
#include "ecs/Zipper.hpp"
#include "scene/SceneManager.hpp"
#include "ecs/Registry.hpp"
#include "network/DataMask.hpp"
#include "input/InputSubsystem.hpp"
#include "network/Action.hpp"
#include "network/Event.hpp"

SecondGame::SecondGame() { 
    m_name = "secondgame"; 
}

void SecondGame::OnEnter() {
    SceneData& data = GetSceneData();
    auto players_list = data.Get<std::vector<std::tuple<uint16_t, bool, std::string>>>(
        "players_list",
        std::vector<std::tuple<uint16_t, bool, std::string>>()
    );
    
    GetRegistry().register_component<Transform>();
    GetRegistry().register_component<RigidBody>();
    GetRegistry().register_component<BoxCollider>();
    GetRegistry().register_component<PlayerEntity>();
    GetRegistry().register_component<InputState>();
    GetRegistry().register_component<FighterState>();
    GetRegistry().register_component<FighterStats>();
    GetRegistry().register_component<HitBox>();
    
    int playerCount = 0;
    for (auto& [playerId, ready, _] : players_list) {
        if (playerCount >= 2) break;
        
        float spawnX = (playerCount == 0) ? 200.0f : 600.0f;
        float spawnY = ARENA_FLOOR;
        
        Entity fighter = GetRegistry().spawn_entity();
        
        GetRegistry().emplace_component<Transform>(
            fighter, 
            Vector2{spawnX, spawnY}, 
            Vector2{2.0f, 2.0f}, 
            0.0f
        );
        
        GetRegistry().emplace_component<RigidBody>(
            fighter,
            1.0f,
            0.0f,
            false
        );
        
        GetRegistry().emplace_component<BoxCollider>(
            fighter,
            30.0f,
            60.0f,
            Vector2{0, 0}
        );
        
        GetRegistry().emplace_component<PlayerEntity>(
            fighter,
            250.0f
        );
        auto& playerComp = GetRegistry().get_components<PlayerEntity>()[fighter];
        playerComp->player_id = playerId;
        playerComp->current = 100;
        playerComp->max = 100;
        playerComp->isAlive = true;
        
        GetRegistry().emplace_component<InputState>(fighter);
        
        GetRegistry().emplace_component<FighterState>(fighter);
        auto& fighterState = GetRegistry().get_components<FighterState>()[fighter];
        fighterState->facingRight = (playerCount == 0);
        
        GetRegistry().emplace_component<FighterStats>(fighter);
        
        GetRegistry().emplace_component<HitBox>(fighter);
        
        m_players[playerId] = fighter;
        roundWins[playerId] = 0;
        playerCount++;
    }
    
    std::cout << "[Fighter] Game initialized with " << m_players.size() << " fighters" << std::endl;
    
    StartNewRound();
}

void SecondGame::OnExit() {
    m_players.clear();
    roundWins.clear();
}

void SecondGame::Update(float deltaTime) {
    ReceivePlayerInputs();
    
    if (roundStartDelay > 0.0f) {
        roundStartDelay -= deltaTime;
        if (roundStartDelay <= 0.0f) {
            roundActive = true;
            std::cout << "[Round " << roundNumber << "] FIGHT!" << std::endl;
        }
        BuildCurrentState();
        return;
    }
    
    if (roundActive) {
        roundTimer -= deltaTime;
        if (roundTimer <= 0.0f) {
            roundTimer = 0.0f;
            CheckRoundEnd();
        }
        
        UpdateFighterStates(deltaTime);
        Movement(deltaTime);
        CheckCombat();
        CheckRoundEnd();
    }
    
    BuildCurrentState();
}

void SecondGame::ReceivePlayerInputs() {
    SceneData& data = GetSceneData();
    
    if (data.Has("last_input_event")) {
        Event event = data.Get<Event>("last_input_event");
        uint16_t playerId = data.Get<uint16_t>("last_input_player");
        
        if (event.type == EventType::PLAYER_INPUT) {
            const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);
            
            auto& players = GetRegistry().get_components<PlayerEntity>();
            auto& states = GetRegistry().get_components<InputState>();
            
            for (auto&& [player, state] : Zipper(players, states)) {
                if (player.player_id != playerId) continue;
                
                state.moveLeft = input.left;
                state.moveRight = input.right;
                state.moveUp = input.up;
                state.action1 = input.fire;
                break;
            }
        }
        
        data.Remove("last_input_event");
        data.Remove("last_input_player");
    }
}

void SecondGame::UpdateFighterStates(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& states = GetRegistry().get_components<InputState>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    auto& fighterStats = GetRegistry().get_components<FighterStats>();
    auto& hitboxes = GetRegistry().get_components<HitBox>();
    
    for (auto&& [transform, rb, player, state, fstate, stats, hitbox] : 
         Zipper(transforms, rigidbodies, players, states, fighterStates, fighterStats, hitboxes)) {
        
        if (!player.isAlive) continue;
        
        fstate.stateTimer += deltaTime;

        for (auto&& [otherTransform, otherPlayer] : Zipper(transforms, players)) {
            if (otherPlayer.player_id == player.player_id) continue;
            fstate.facingRight = (otherTransform.position.x > transform.position.x);
        }
        
        switch (fstate.currentState) {
            case FighterState::IDLE:
                hitbox.active = false;
                
                if (!stats.isGrounded) {
                    fstate.currentState = FighterState::JUMPING;
                } else if (state.action1) {
                    fstate.currentState = FighterState::ATTACKING_LIGHT;
                    fstate.stateTimer = 0.0f;
                } else if (state.action2) {
                    fstate.currentState = FighterState::ATTACKING_HEAVY;
                    fstate.stateTimer = 0.0f;
                } else if (state.moveLeft || state.moveRight) {
                    fstate.currentState = FighterState::WALKING;
                } else if (state.moveUp && stats.isGrounded) {
                    rb.velocity.y = -stats.jumpForce;
                    stats.isGrounded = false;
                    fstate.currentState = FighterState::JUMPING;
                }
                break;
                
            case FighterState::WALKING:
                hitbox.active = false;
                
                if (state.moveLeft && transform.position.x > ARENA_LEFT) {
                    rb.velocity.x = -stats.moveSpeed;
                } else if (state.moveRight && transform.position.x < ARENA_RIGHT) {
                    rb.velocity.x = stats.moveSpeed;
                } else {
                    rb.velocity.x = 0;
                }
                
                if (!state.moveLeft && !state.moveRight) {
                    fstate.currentState = FighterState::IDLE;
                } else if (state.action1 || state.action2) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::JUMPING:
                hitbox.active = false;
                
                if (state.moveLeft && transform.position.x > ARENA_LEFT) {
                    rb.velocity.x = -stats.moveSpeed * 0.5f;
                } else if (state.moveRight && transform.position.x < ARENA_RIGHT) {
                    rb.velocity.x = stats.moveSpeed * 0.5f;
                }
                
                if (stats.isGrounded) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::ATTACKING_LIGHT:
                rb.velocity.x = 0;
                
                if (fstate.stateTimer < 0.1f) {
                    hitbox.active = false;
                } else if (fstate.stateTimer < 0.2f) {
                    hitbox.active = true;
                    hitbox.damage = 10;
                    hitbox.stunDuration = 0.2f;
                    hitbox.width = 40.0f;
                    hitbox.height = 30.0f;
                    hitbox.offset = fstate.facingRight ? Vector2{30, 0} : Vector2{-30, 0};
                } else if (fstate.stateTimer >= 0.3f) {
                    hitbox.active = false;
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
                
            case FighterState::ATTACKING_HEAVY:
                rb.velocity.x = 0;
                
                if (fstate.stateTimer < 0.25f) {
                    hitbox.active = false;
                } else if (fstate.stateTimer < 0.4f) {
                    hitbox.active = true;
                    hitbox.damage = 25;
                    hitbox.stunDuration = 0.6f;
                    hitbox.width = 60.0f;
                    hitbox.height = 50.0f;
                    hitbox.offset = fstate.facingRight ? Vector2{40, 0} : Vector2{-40, 0};
                } else if (fstate.stateTimer >= 0.7f) {
                    hitbox.active = false;
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
        }
    }
}

void SecondGame::Movement(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& stats = GetRegistry().get_components<FighterStats>();
    
    const Vector2 gravity = {0, 980.0f};
    
    for (auto&& [transform, rb, stat] : Zipper(transforms, rigidbodies, stats)) {
        rb.velocity.y += gravity.y * deltaTime;
        
        transform.position += rb.velocity * deltaTime;
        
        if (transform.position.y >= ARENA_FLOOR) {
            transform.position.y = ARENA_FLOOR;
            rb.velocity.y = 0;
            stat.isGrounded = true;
        } else {
            stat.isGrounded = false;
        }
        
        if (transform.position.x < ARENA_LEFT) {
            transform.position.x = ARENA_LEFT;
            rb.velocity.x = 0;
        } else if (transform.position.x > ARENA_RIGHT) {
            transform.position.x = ARENA_RIGHT;
            rb.velocity.x = 0;
        }
    }
}

void SecondGame::CheckCombat() {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    auto& hitboxes = GetRegistry().get_components<HitBox>();
    
    std::vector<Entity> fighters;
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].has_value()) {
            fighters.push_back(GetRegistry().entity_from_index(i));
        }
    }
    
    for (size_t i = 0; i < fighters.size(); ++i) {
        for (size_t j = i + 1; j < fighters.size(); ++j) {
            Entity attacker = fighters[i];
            Entity defender = fighters[j];
            
            auto& attackerHitbox = hitboxes[attacker].value();
            auto& attackerTransform = transforms[attacker].value();
            
            auto& defenderTransform = transforms[defender].value();
            auto& defenderPlayer = players[defender].value();
            auto& defenderFState = fighterStates[defender].value();
            
            if (!attackerHitbox.active) continue;
            
            Vector2 hitboxPos = attackerTransform.position + attackerHitbox.offset;
            
            float dx = std::abs(hitboxPos.x - defenderTransform.position.x);
            float dy = std::abs(hitboxPos.y - defenderTransform.position.y);
            
            if (dx < (attackerHitbox.width / 2 + 15) && 
                dy < (attackerHitbox.height / 2 + 30)) {

                defenderPlayer.current -= attackerHitbox.damage;
                
                std::cout << "[Combat] Player " << defenderPlayer.player_id 
                          << " hit for " << attackerHitbox.damage 
                          << " damage! HP: " << defenderPlayer.current << std::endl;
                
                attackerHitbox.active = false;
                
                if (defenderPlayer.current <= 0) {
                    defenderPlayer.current = 0;
                    defenderPlayer.isAlive = false;
                }
            }
        }
    }
}

void SecondGame::CheckRoundEnd() {
    SceneData& data = GetSceneData();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    
    int alivePlayers = 0;
    uint16_t winnerId = 0;
    
    for (auto&& [player] : Zipper(players)) {
        if (player.isAlive) {
            alivePlayers++;
            winnerId = player.player_id;
        }
    }
    
    if (alivePlayers <= 1 || roundTimer <= 0.0f) {
        roundActive = false;
        
        if (alivePlayers == 1) {
            roundWins[winnerId]++;
            std::cout << "[Round " << roundNumber << "] Player " << winnerId << " wins!" << std::endl;
        } else {
            uint16_t healthWinner = 0;
            int maxHealth = -1;
            
            for (auto&& [player] : Zipper(players)) {
                if (player.current > maxHealth) {
                    maxHealth = player.current;
                    healthWinner = player.player_id;
                }
            }
            
            if (maxHealth > 0) {
                roundWins[healthWinner]++;
                std::cout << "[Round " << roundNumber << "] Time out! Player " 
                          << healthWinner << " wins by health!" << std::endl;
            }
        }
        
        for (auto& [playerId, wins] : roundWins) {
            if (wins >= 2) {
                std::cout << "[Match] Player " << playerId << " wins the match!" << std::endl;
                data.Set("game_ended", true);
                data.Set("victory", true);
                data.Set("winner_id", playerId);
                return;
            }
        }
        
        roundNumber++;
        if (roundNumber <= MAX_ROUNDS) {
            StartNewRound();
        }
    }
}

void SecondGame::StartNewRound() {
    roundTimer = 99.0f;
    roundStartDelay = 3.0f;
    roundActive = false;
    
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    
    int playerIndex = 0;
    for (auto&& [transform, rb, player, fstate] : 
         Zipper(transforms, rigidbodies, players, fighterStates)) {
        
        float spawnX = (playerIndex == 0) ? 200.0f : 600.0f;
        transform.position = Vector2{spawnX, ARENA_FLOOR};
        rb.velocity = {0, 0};
        
        player.current = player.max;
        player.isAlive = true;
        fstate.currentState = FighterState::IDLE;
        fstate.stateTimer = 0.0f;
        
        playerIndex++;
    }
    
    std::cout << "[Round " << roundNumber << "] Ready..." << std::endl;
}

void SecondGame::BuildCurrentState() {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    
    GameState gs;
    
    for (auto&& [idx, player, transform, fstate] : 
         IndexedZipper(players, transforms, fighterStates)) {
        
        Entity e = GetRegistry().entity_from_index(idx);
        if (!GetRegistry().is_entity_valid(e)) continue;
        
        PlayerState ps;
        ps.playerId = player.player_id;
        ps.posX = transform.position.x;
        ps.posY = transform.position.y;
        ps.hp = static_cast<uint8_t>(player.current);
        ps.state = static_cast<uint8_t>(fstate.currentState);
        ps.sprite = fstate.facingRight ? 1 : 0;
        ps.score = roundWins[player.player_id];
        
        ps.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_SPRITE | M_SCORE;
        
        gs.players.push_back(ps);
    }
    
    SceneData& data = GetSceneData();
    data.Set("game_state", gs);
}

void SecondGame::HandleEvent(SDL_Event& event) {}

extern "C" {
    Scene* CreateScene() {
        return new SecondGame();
    }
}
